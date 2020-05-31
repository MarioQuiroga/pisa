#include <algorithm>
#include <iostream>
#include <numeric>
#include <optional>
#include <string>
#include <fstream>

#define __QUERIES_CACHING__

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <mio/mmap.hpp>
#include <range/v3/view/enumerate.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include "mappable/mapper.hpp"

#include "index_types.hpp"
#include "accumulator/lazy_accumulator.hpp"
#include "query/queries.hpp"
#include "timer.hpp"
#include "util/util.hpp"
#include "wand_data_compressed.hpp"
#include "wand_data_raw.hpp"
#include "cursor/cursor.hpp"
#include "cursor/scored_cursor.hpp"
#include "cursor/max_scored_cursor.hpp"
#include "cursor/block_max_scored_cursor.hpp"

#include "CLI/CLI.hpp"

#include "cache_policies/lru_policy.hpp"
#include "cache_policies/lfu_policy.hpp"
#include "cache_policies/lfuda_policy.hpp"
#include "caching_files/Cache.hpp"

using namespace pisa;
using ranges::view::enumerate;

enum policy_type{
    lru_cache,
    lfu_cache,
    lfuda_cache
};

template <typename Fn>
void extract_times(Fn fn,
                   std::vector<Query> const &queries,
                   std::string const &index_type,
                   std::string const &query_type,
                   size_t runs,
                   std::ostream &os)
{
    std::vector<std::size_t> times(runs);
    for (auto &&[qid, query] : enumerate(queries)) {
        do_not_optimize_away(fn(query));
        std::generate(times.begin(), times.end(), [&fn, &q = query]() {
            return run_with_timer<std::chrono::microseconds>(
                       [&]() { do_not_optimize_away(fn(q)); })
                .count();
        });
        auto mean = std::accumulate(times.begin(), times.end(), std::size_t{0}, std::plus<>()) / runs;
        os << fmt::format("{}\t{}\n", query.id.value_or(std::to_string(qid)), mean);
    }
}

template <typename Functor>
void op_perftest(Functor query_func,
                 std::vector<Query> const &queries,
                 std::string const &index_type,
                 std::string const &query_type,
                 size_t runs)
{

    std::vector<double> query_times;
    //std::cout << runs << std::endl;
    for (size_t run = 0; run <= runs; ++run) {
        //std::cout << queries.size() << std::endl;
        for (auto const &query : queries) {
            auto usecs = run_with_timer<std::chrono::microseconds>([&]() {
                uint64_t result = query_func(query);
                do_not_optimize_away(result);
            });
            if (run != 0) { // first run is not timed
                query_times.push_back(usecs.count());
                //std::cout << "qqqqqqqq" << std::endl;
            }
        }
    }
    //std::cout << query_times.size() << std::endl;
    if (false) {
        for (auto t : query_times) {
            std::cout << (t / 1000) << std::endl;
        }
    } else {
        std::sort(query_times.begin(), query_times.end());
        double avg =
            std::accumulate(query_times.begin(), query_times.end(), double()) / query_times.size();
        double total_time =
            std::accumulate(query_times.begin(), query_times.end(), double());
        double q50 = query_times[query_times.size() / 2];
        double q90 = query_times[90 * query_times.size() / 100];
        double q95 = query_times[95 * query_times.size() / 100];

        spdlog::info("Index type {}", index_type);
        spdlog::info("Mean: {}", avg);
        spdlog::info("50% quantile: {}", q50);
        spdlog::info("90% quantile: {}", q90);
        spdlog::info("95% quantile: {}", q95);
        spdlog::info("Total time: {}", total_time);
        spdlog::info("Total querys: {}", query_times.size());
        //stats_line()("type", index_type)("query", query_type)("avg", avg)("q50", q50)("q90", q90)(
        //    "q95", q95);
    }
}

template <typename IndexType, template<typename, class, typename> class Policy>
struct add_policy { typedef IndexType type; };

template <typename BlockType, template<typename, class, typename> class Policy>
struct add_policy<block_freq_index<BlockType, false, Policy>, Policy> {
    typedef block_freq_index<BlockType, false, Policy> type;
}; 

template <typename IndexType, typename WandType, template<typename, class, typename> class Policy>
void perftest(const std::string &index_filename,
              const std::optional<std::string> &wand_data_filename,
              const std::vector<Query> &queries,
              const std::optional<std::string> &thresholds_filename,
              std::string const &type,
              std::string const &query_type,
              uint64_t k,
              bool extract, std::string const &policy, uint64_t size)
{
    typename add_policy<IndexType, Policy>::type index;
    //IndexType index;
    spdlog::info("Loading index from {}", index_filename);
    mio::mmap_source m(index_filename.c_str());
    mapper::map(index, m);
    std::cout << "Init Cache" << std::endl;
    index.init_cache(size);

    spdlog::info("Warming up posting lists");
    std::unordered_set<term_id_type> warmed_up;
    for (auto const &q : queries) {
        for (auto t : q.terms) {
            if (!warmed_up.count(t)) {
                index.warmup(t);
                warmed_up.insert(t);
            }
        }
    }

    WandType wdata;

    std::vector<std::string> query_types;
    boost::algorithm::split(query_types, query_type, boost::is_any_of(":"));
    mio::mmap_source md;
    if (wand_data_filename) {
        std::error_code error;
        md.map(*wand_data_filename, error);
        if(error){
            std::cerr << "error mapping file: " << error.message() << ", exiting..." << std::endl;
            throw std::runtime_error("Error opening file");
        }
        mapper::map(wdata, md, mapper::map_flags::warmup);
    }

    std::vector<float> thresholds;
    if (thresholds_filename) {
        std::string t;
        std::ifstream tin(*thresholds_filename);
        while (std::getline(tin, t)) {
            thresholds.push_back(std::stof(t));
        }
    }

    spdlog::info("Performing {} queries", type);
    spdlog::info("K: {}", k);

    for (auto &&t : query_types) {
        spdlog::info("Query type: {}", t);
        std::function<uint64_t(Query)> query_fun;
        if (t == "and") {
            query_fun = [&](Query query){
                and_query<false> and_q;
                return and_q(make_scored_cursors(index, wdata, query), index.num_docs()).size();
            };
        } else if (t == "and_freq") {
            query_fun = [&](Query query){
                and_query<true> and_q;
                return and_q(make_scored_cursors(index, wdata, query), index.num_docs()).size();
            };
        } else if (t == "or") {
            query_fun = [&](Query query){
                or_query<false> or_q;
                return or_q(make_cursors(index, query), index.num_docs());
            };
        } else if (t == "or_freq") {
            query_fun = [&](Query query){
                or_query<true> or_q;
                return or_q(make_cursors(index, query), index.num_docs());
            };
        } else if (t == "wand" && wand_data_filename) {
            query_fun = [&](Query query){
                wand_query wand_q(k);
                return wand_q(make_max_scored_cursors(index, wdata, query), index.num_docs());
            };
        } else if (t == "block_max_wand" && wand_data_filename) {
            query_fun = [&](Query query){
                block_max_wand_query block_max_wand_q(k);
                return block_max_wand_q(make_block_max_scored_cursors(index, wdata, query), index.num_docs());
            };
        } else if (t == "block_max_maxscore" && wand_data_filename) {
            query_fun = [&](Query query){
                block_max_maxscore_query block_max_maxscore_q(k);
                return block_max_maxscore_q(make_block_max_scored_cursors(index, wdata, query), index.num_docs());
            };
        }  else if (t == "ranked_and" && wand_data_filename) {
            query_fun = [&](Query query){
                ranked_and_query ranked_and_q(k);
                return ranked_and_q(make_scored_cursors(index, wdata, query), index.num_docs());
            };
        } else if (t == "block_max_ranked_and" && wand_data_filename) {
            query_fun = [&](Query query){
                block_max_ranked_and_query block_max_ranked_and_q(k);
                return block_max_ranked_and_q(make_block_max_scored_cursors(index, wdata, query), index.num_docs());
            };
        }  else if (t == "ranked_or" && wand_data_filename) {
            query_fun = [&](Query query){
                ranked_or_query ranked_or_q(k);
                return ranked_or_q(make_scored_cursors(index, wdata, query), index.num_docs());
            };
        } else if (t == "maxscore" && wand_data_filename) {
            query_fun = [&](Query query){
                maxscore_query maxscore_q(k);
                return maxscore_q(make_max_scored_cursors(index, wdata, query), index.num_docs());
            };
        } else if (t == "ranked_or_taat" && wand_data_filename) {
            Simple_Accumulator accumulator(index.num_docs());
            ranked_or_taat_query ranked_or_taat_q(k);
            query_fun = [&, ranked_or_taat_q, accumulator](Query query) mutable {
                return ranked_or_taat_q(make_scored_cursors(index, wdata, query), index.num_docs(), accumulator);
            };
        } else if (t == "ranked_or_taat_lazy" && wand_data_filename) {
            Lazy_Accumulator<4> accumulator(index.num_docs());
            ranked_or_taat_query ranked_or_taat_q(k);
            query_fun = [&, ranked_or_taat_q, accumulator](Query query) mutable {
                return ranked_or_taat_q(make_scored_cursors(index, wdata, query), index.num_docs(), accumulator);
            };
        } else {
            spdlog::error("Unsupported query type: {}", t);
            break;
        }
        if (extract) {
            extract_times(query_fun, queries, type, t, 2, std::cout);
        } else {
            op_perftest(query_fun, queries, type, t, 2);            
            spdlog::info("Policy: {}", policy);
            spdlog::info("Hit Ratio: {}", index.get_hit_ratio());
        }
    }
}

using wand_raw_index = wand_data<wand_data_raw>;
using wand_uniform_index = wand_data<wand_data_compressed>;

int main(int argc, const char **argv)
{
    std::string type;
    std::string query_type;
    std::string index_filename;
    std::optional<std::string> terms_file;
    std::optional<std::string> wand_data_filename;
    std::optional<std::string> query_filename;
    std::optional<std::string> thresholds_filename;
    std::optional<std::string> stopwords_filename;
    std::optional<std::string> stemmer = std::nullopt;
    std::string policy;
    uint64_t k = configuration::get().k;
    uint64_t size_cache;
    uint64_t queries_size;
    bool compressed = false;
    bool extract = false;
    bool silent = false;

    CLI::App app{"queries - a tool for performing queries on an index."};
    app.set_config("--config", "", "Configuration .ini file", false);
    app.add_option("-t,--type", type, "Index type")->required();
    app.add_option("-a,--algorithm", query_type, "Query algorithm")->required();
    app.add_option("-i,--index", index_filename, "Collection basename")->required();
    app.add_option("-w,--wand", wand_data_filename, "Wand data filename");
    app.add_option("-q,--query", query_filename, "Queries filename");
    app.add_option("--queries_size", queries_size, "Queries size")->required();
    app.add_flag("--compressed-wand", compressed, "Compressed wand input file");
    app.add_option("-k", k, "k value");
    app.add_option("-p", policy, "policy")->required();
    app.add_option("-s", size_cache, "size_cache")->required();
    app.add_option("-T,--thresholds", thresholds_filename, "k value");
    auto *terms_opt = app.add_option("--terms", terms_file, "Term lexicon");
    app.add_option("--stopwords", stopwords_filename, "File containing stopwords to ignore")->needs(terms_opt);
    app.add_option("--stemmer", stemmer, "Stemmer type")->needs(terms_opt);
    app.add_flag("--extract", extract, "Extract individual query times");
    app.add_flag("--silent", silent, "Suppress logging");
    CLI11_PARSE(app, argc, argv);

    if (silent) {
        spdlog::set_default_logger(spdlog::create<spdlog::sinks::null_sink_mt>("stderr"));
    } else {
        spdlog::set_default_logger(spdlog::stderr_color_mt("stderr"));
    }
    if (extract) {
        std::cout << "qid\tusec\n";
    }
    std::cout << "Parsing queries..." << std::endl;

   


    std::vector<Query> myVec;
    auto parse_query = resolve_query_parser(myVec, terms_file, stopwords_filename, stemmer);
    if (query_filename) {
        std::ifstream is(*query_filename);
        io::for_each_line(is, parse_query);
    } else {
        io::for_each_line(std::cin, parse_query);
    }
    std::cout << "Queries size file: " << myVec.size() << std::endl;
    std::vector<Query> queries(myVec.begin(), myVec.begin() + queries_size);
    std::cout << "Queries size: " << queries.size() << std::endl;

    ofstream myfile;
    myfile.open ("queries_sample.txt");

    for (auto const &q : queries) {
        for (auto t : q.terms) {
            myfile << t << " ";
        }
        myfile << std::endl;
    }
    myfile.close();
    
    /**/
    if (false) {
#define LOOP_BODY(R, DATA, T)                                                          \
    }                                                                                  \
    else if (type == BOOST_PP_STRINGIZE(T))                                            \
    {                                                                                   \
        if (policy.compare("lru")==0){                                                                                  \
            if (compressed) {                                                              \
                perftest<BOOST_PP_CAT(T, _index), wand_uniform_index, Policies::lru_cache>(index_filename,      \
                                                                    wand_data_filename,  \
                                                                    queries,             \
                                                                    thresholds_filename, \
                                                                    type,                \
                                                                    query_type,          \
                                                                    k,                   \
                                                                    extract, policy, size_cache);            \
            } else {                                                                       \
                perftest<BOOST_PP_CAT(T, _index), wand_raw_index, Policies::lru_cache>(index_filename,          \
                                                                wand_data_filename,      \
                                                                queries,                 \
                                                                thresholds_filename,     \
                                                                type,                    \
                                                                query_type,              \
                                                                k,                       \
                                                                extract, policy, size_cache);                \
            }                                                                          \
        }                                                                              \
        if (policy.compare("lfu")==0){                                                   \
            if (compressed) {                                                              \
                perftest<BOOST_PP_CAT(T, _index), wand_uniform_index, Policies::lfu_cache>(index_filename,      \
                                                                    wand_data_filename,  \
                                                                    queries,             \
                                                                    thresholds_filename, \
                                                                    type,                \
                                                                    query_type,          \
                                                                    k,                   \
                                                                    extract, policy, size_cache);            \
            } else {                                                                       \
                perftest<BOOST_PP_CAT(T, _index), wand_raw_index, Policies::lfu_cache>(index_filename,          \
                                                                wand_data_filename,      \
                                                                queries,                 \
                                                                thresholds_filename,     \
                                                                type,                    \
                                                                query_type,              \
                                                                k,                       \
                                                                extract, policy, size_cache);                \
            }                                                                          \
        }                                                                               \
        if (policy.compare("lfuda")==0){                                                   \
            if (compressed) {                                                              \
                perftest<BOOST_PP_CAT(T, _index), wand_uniform_index, Policies::lfuda_cache>(index_filename,      \
                                                                    wand_data_filename,  \
                                                                    queries,             \
                                                                    thresholds_filename, \
                                                                    type,                \
                                                                    query_type,          \
                                                                    k,                   \
                                                                    extract, policy, size_cache);            \
            } else {                                                                       \
                perftest<BOOST_PP_CAT(T, _index), wand_raw_index, Policies::lfuda_cache>(index_filename,          \
                                                                wand_data_filename,      \
                                                                queries,                 \
                                                                thresholds_filename,     \
                                                                type,                    \
                                                                query_type,              \
                                                                k,                       \
                                                                extract, policy, size_cache);                \
            }                                                                          \
        }                                                                               \
        /**/

        BOOST_PP_SEQ_FOR_EACH(LOOP_BODY, _, PISA_INDEX_TYPES);
#undef LOOP_BODY

    } else {
        spdlog::error("Unknown type {}", type);
    }
}