#pragma once

#include <omp.h>
#include <type_traits>

#include "geometry/point_containers.hpp"
#include "kernels/kernel_factory.hpp"
#include "structures/linear_octree.hpp"
#include "structures/nanoflann.hpp"
#include "structures/nanoflann_wrappers.hpp"
#include "structures/octree.hpp"
#include "structures/unibn_octree.hpp"
#include "structures/octree_reordered.hpp"
#include "structures/octree_range_selector.hpp"

#include "benchmarking.hpp"
#include "papi_events.hpp"
#include "time_watcher.hpp"
#include "search_set.hpp"
#include "main_options.hpp"

#ifdef HAVE_PICOTREE
#include "structures/picotree_wrappers.hpp"
#endif

#ifdef HAVE_PCL
#include <pcl/point_cloud.h>
#include <pcl/octree/octree_search.h>
#include <pcl/kdtree/kdtree_flann.h>
#include "structures/pcl_wrappers.hpp"
#endif

template <PointContainer Container>
class NeighborsBenchmark {
    private:
        using PointEncoder = PointEncoding::PointEncoder;
        using key_t = PointEncoding::key_t;
        using coords_t = PointEncoding::coords_t;
        PointEncoder& enc;
        std::vector<key_t>& codes;
        Box box;
        const std::string comment;
        Container& points;
        size_t currentBenchmarkExecution = 0;
        std::ofstream &outputFile;
        SearchSet &searchSet;
        
        /**
         * main_parameter might be radius (fixed-radius searches) or k (knn searches)
         * in any case, we write it on radius column in the csv, for simplicity
         * kernel is "kNN" or one of the 4 kernels for radius searches
         */
        template <typename ParameterType>
        inline void appendToCsv(SearchAlgo algo, std::string_view kernel, ParameterType main_parameter, const benchmarking::Stats<>& stats, 
                                std::string_view reorderMode = "none", size_t averageResultSize = 0,
                                int numThreads = omp_get_max_threads(), double tolerancePercentage = 0.0) {
            // Check if the file is empty and append header if it is
            if (outputFile.tellp() == 0) {
                outputFile <<   "date,octree,point_type,encoder,npoints,operation,kernel,radius,reorder,num_searches,sequential_searches,repeats,"
                                "accumulated,mean,median,stdev,used_warmup,warmup_time,avg_result_size,tolerance_percentage,"
                                "openmp_threads,openmp_schedule\n";
            }

            // if the comment, exists, append it to the op. name
            std::string fullAlgoName = std::string(searchAlgoToString(algo)) + ((comment != "") ? "_" + comment : "");
            
            // Get OpenMP runtime information
            omp_sched_t openmpSchedule;
            int openmpChunkSize;
            omp_get_schedule(&openmpSchedule, &openmpChunkSize);
            std::string openmpScheduleName;
            switch (openmpSchedule) {
                case omp_sched_static: openmpScheduleName = "static"; break;
                case omp_sched_dynamic: openmpScheduleName = "dynamic"; break;
                case omp_sched_guided: openmpScheduleName = "guided"; break;
                default: openmpScheduleName = "unknown"; break;
            }
            std::string sequentialSearches;
            if(searchSet.sequential) {
                sequentialSearches = "sequential";
            } else {
                sequentialSearches = "random";
            }
            outputFile << getCurrentDate() << ',' 
                << searchStructureToString(algoToStructure(algo)) << ',' 
                << containerTypeToString(mainOptions.containerType) << ','
                << enc.getEncoderName() << ','
                << points.size() <<  ','
                << fullAlgoName << ',' 
                << kernel << ',' 
                << main_parameter << ','
                << reorderMode << ','
                << searchSet.numSearches << ',' 
                << sequentialSearches << ','
                << stats.size() << ','
                << stats.accumulated() << ',' 
                << stats.mean() << ',' 
                << stats.median() << ',' 
                << stats.stdev() << ','
                << stats.usedWarmup() << ','
                << stats.warmupValue() << ','
                << averageResultSize << ','
                << tolerancePercentage << ','
                << numThreads << ','
                << openmpScheduleName
                << std::endl;
        }
        template <typename ParameterType>
        inline void appendToCsv(SearchAlgo algo, std::string_view kernel, ParameterType main_parameter, const benchmarking::Stats<>& stats, 
                                std::vector<long long> &eventValues, std::string_view reorderMode = "none",
                                size_t averageResultSize = 0, 
                                int numThreads = omp_get_max_threads(), double tolerancePercentage = 0.0
                                ) {
            // Check if the file is empty and append header if it is
            if (outputFile.tellp() == 0) {
                outputFile <<   "date,octree,point_type,encoder,npoints,operation,kernel,radius,reorder,num_searches,sequential_searches,repeats,"
                                "accumulated,mean,median,stdev,used_warmup,warmup_time,avg_result_size,tolerance_percentage,"
                                "openmp_threads,openmp_schedule,l1d_miss,l2d_miss,l3_miss\n";
            }

            // if the comment, exists, append it to the op. name
            std::string fullAlgoName = std::string(searchAlgoToString(algo)) + ((comment != "") ? "_" + comment : "");
            
            // Get OpenMP runtime information
            omp_sched_t openmpSchedule;
            int openmpChunkSize;
            omp_get_schedule(&openmpSchedule, &openmpChunkSize);
            std::string openmpScheduleName;
            switch (openmpSchedule) {
                case omp_sched_static: openmpScheduleName = "static"; break;
                case omp_sched_dynamic: openmpScheduleName = "dynamic"; break;
                case omp_sched_guided: openmpScheduleName = "guided"; break;
                default: openmpScheduleName = "unknown"; break;
            }
            std::string sequentialSearches;
            if(searchSet.sequential) {
                sequentialSearches = "sequential";
            } else {
                sequentialSearches = "random";
            }
            outputFile << getCurrentDate() << ',' 
                << searchStructureToString(algoToStructure(algo)) << ',' 
                << containerTypeToString(mainOptions.containerType) << ','
                << enc.getEncoderName() << ','
                << points.size() <<  ','
                << fullAlgoName << ',' 
                << kernel << ',' 
                << main_parameter << ','
                << reorderMode << ','
                << searchSet.numSearches << ',' 
                << sequentialSearches << ','
                << stats.size() << ','
                << stats.accumulated() << ',' 
                << stats.mean() << ',' 
                << stats.median() << ',' 
                << stats.stdev() << ','
                << stats.usedWarmup() << ','
                << stats.warmupValue() << ','
                << averageResultSize << ','
                << tolerancePercentage << ','
                << numThreads << ','
                << openmpScheduleName << ','
                << eventValues[0] << ','
                << eventValues[1] << ','
                << eventValues[2]
                << std::endl;
        }

        template <typename OctreeT, typename ReorderedT>
        void debugRangeSelector(const OctreeT& oct, const ReorderedT& reordered, ReorderMode mode, Kernel_t kernel) {
            std::cout << "[LOG] Benchmarking range selector for " << kernelToString(kernel) << std::endl;

            if (mode == ReorderMode::None) {
                return;
            }

            const size_t numLeavesToTest = std::min(size_t(15), oct.getNumLeaves());
            for (size_t r = 0; r < mainOptions.benchmarkRadii.size(); r++) {
                double testRadius = mainOptions.benchmarkRadii[r];

                for (size_t leaf = 0; leaf < numLeavesToTest; ++leaf) {
                    std::vector<size_t> leafPoints;
                    size_t begin = 0;
                    size_t end = 0;
                    size_t count = 0;
                    if constexpr (requires { oct.getLeafPoints(leaf); }) {
                        leafPoints = oct.getLeafPoints(leaf);
                        count = leafPoints.size();
                    } else {
                        const auto range = oct.getLeafRange(leaf);
                        begin = range.first;
                        end = range.second;
                        count = end - begin;
                    }
                    if (count == 0) continue;

                    size_t queryIdx = begin;
                    if constexpr (requires { oct.getLeafPoints(leaf); }) {
                        queryIdx = leafPoints[0];
                    }

                    // Usamos el primer punto de la hoja como query de prueba
                    Point testQuery;
                    if constexpr (std::is_same_v<Container, PointsSoA>) {
                        testQuery = Point(points.dataX()[queryIdx], points.dataY()[queryIdx], points.dataZ()[queryIdx]);
                    } else {
                        testQuery = points[queryIdx];
                    }

                    PrunedRange range = bestRange(
                        leaf,
                        testQuery,
                        testRadius,
                        kernel,
                        oct,
                        points,
                        reordered,
                        mode,
                        false);

                    const double prunedPct = (count > 0)
                        ? 100.0 * (1.0 - static_cast<double>(range.count()) / count)
                        : 0.0;
                    if(prunedPct > 0.01) {
                        std::cout << "[LOG]   leaf=" << leaf
                                << " count=" << count
                                << " bestOrder=K" << static_cast<int>(range.order)
                                << " range=[" << range.iMin << "," << range.iMax << ")"
                                << " kept=" << range.count()
                                << " pruned=" << std::fixed << std::setprecision(1) << prunedPct << "%"
                                << "\n"
                                << std::endl;
                    }
                }
            }
        }


    public:
        NeighborsBenchmark(Container& points, std::vector<key_t>& codes, Box box, PointEncoder& enc, SearchSet& searchSet, std::ofstream &file) :
            points(points), 
            codes(codes),
            box(box),
            enc(enc),
            searchSet(searchSet),
            outputFile(file) {}

        void printBenchmarkInfo() {
            std::cout << std::fixed << std::setprecision(3);
            std::cout << std::left << "Starting neighbor search benchmark!\n";
            std::cout << std::left << std::setw(LOG_FIELD_WIDTH) << "Encoder:"                    << std::setw(LOG_FIELD_WIDTH)   << enc.getEncoderName()                               << "\n";
            std::cout << std::left << std::setw(LOG_FIELD_WIDTH) << "Number of searches per run:" << std::setw(LOG_FIELD_WIDTH)   << searchSet.numSearches                              << "\n";
            std::cout << std::left << std::setw(LOG_FIELD_WIDTH) << "Repeats:"                    << std::setw(LOG_FIELD_WIDTH)   << mainOptions.repeats                                << "\n";
            std::cout << std::left << std::setw(LOG_FIELD_WIDTH) << "Warmup:"                     << std::setw(LOG_FIELD_WIDTH)   << (mainOptions.useWarmup ? "enabled" : "disabled")               << "\n";
            std::cout << std::left << std::setw(LOG_FIELD_WIDTH) << "Search set distribution:"    << std::setw(LOG_FIELD_WIDTH)   << (searchSet.sequential ? "sequential" : "random")   << "\n";
            std::cout << std::endl;
        }
        
        void executeBenchmark(const std::function<size_t(double)>& searchCallback, std::string_view kernelName, SearchAlgo algo,
                      std::string_view reorderMode = "none") {
            std::cout << "  Running " << searchAlgoToString(algo) << " on kernel " << kernelName << std::endl;
            const auto& radii = mainOptions.benchmarkRadii;
            const size_t repeats = mainOptions.repeats;
            const auto& numThreads = mainOptions.numThreads;         
            for (size_t th = 0; th < numThreads.size(); th++) {    
                size_t numberOfThreads = numThreads[th];                
                omp_set_num_threads(numberOfThreads);
                for (size_t r = 0; r < radii.size(); r++) {
                    double radius = radii[r];
                    if(mainOptions.cacheProfiling) {
                        auto [events, descriptions] = buildCombinedEventList();
                        int eventSet = initPapiEventSet(events);
                        std::vector<long long> eventValues(events.size());
                        if (eventSet == PAPI_NULL) {
                            std::cout << "Failed to initialize PAPI event set." << std::endl;
                            exit(1);
                        }
                        auto [stats, averageResultSize] = benchmarking::benchmark<size_t>(repeats, [&]() { 
                            return searchCallback(radius); 
                        }, mainOptions.useWarmup, eventSet, eventValues.data());
                        printPapiResults(events, descriptions, eventValues);
                        appendToCsv(algo, kernelName, radius, stats, eventValues, reorderMode, averageResultSize, numberOfThreads);
                    } else {
                        auto [stats, averageResultSize] = benchmarking::benchmark<size_t>(repeats, [&]() { 
                            return searchCallback(radius); 
                        }, mainOptions.useWarmup);
                        appendToCsv(algo, kernelName, radius, stats, reorderMode, averageResultSize, numberOfThreads);
                    }
                    searchSet.reset();
                    std::cout << std::setprecision(2);
                    std::cout << "    (" << r + th*numThreads.size() + 1 << "/" << numThreads.size() * radii.size() << ") " 
                        << "Radius  " << std::setw(8) << radius 
                        << "Threads " << std::setw(8) << numberOfThreads
                        << std::endl;
                }
            }
        }

        void executeKNNBenchmark(const std::function<size_t(size_t)>& knnSearchCallback, SearchAlgo algo) {
            std::cout << "  Running k-NN searches with " << searchAlgoToString(algo) << std::endl;
            const auto& kValues = mainOptions.benchmarkKValues;
            const size_t repeats = mainOptions.repeats;
            const auto& numThreads = mainOptions.numThreads;         
            for (size_t th = 0; th < numThreads.size(); th++) {    
                size_t numberOfThreads = numThreads[th];                
                omp_set_num_threads(numberOfThreads);
                for (size_t i = 0; i < kValues.size(); i++) {
                    size_t k = kValues[i];
                    if(mainOptions.cacheProfiling) {
                        auto [events, descriptions] = buildCombinedEventList();
                        int eventSet = initPapiEventSet(events);
                        std::vector<long long> eventValues(events.size());
                        if (eventSet == PAPI_NULL) {
                            std::cout << "Failed to initialize PAPI event set." << std::endl;
                            exit(1);
                        }
                        auto [stats, averageResultSize] = benchmarking::benchmark<size_t>(repeats, [&]() { 
                            return knnSearchCallback(k); 
                        }, mainOptions.useWarmup, eventSet, eventValues.data());
                        printPapiResults(events, descriptions, eventValues);
                        appendToCsv(algo, "KNN", k, stats, eventValues, "none", averageResultSize, numberOfThreads);
                    } else {
                        auto [stats, averageResultSize] = benchmarking::benchmark<size_t>(repeats, [&]() { 
                            return knnSearchCallback(k); 
                        }, mainOptions.useWarmup);
                        appendToCsv(algo, "KNN", k, stats, "none", averageResultSize, numberOfThreads);
                    }
                    searchSet.reset();
                    std::cout << std::setprecision(2);
                    std::cout << "    (" << i + th*numThreads.size() + 1 << "/" << numThreads.size() * kValues.size() << ") " 
                        << "k  " << std::setw(8) << k 
                        << "Threads " << std::setw(8) << numberOfThreads
                        << std::endl;
                }
            }
        }

        void benchmarkNanoflannKDTree(NanoFlannKDTree<Container> &kdtree, std::string_view kernelName) {
            if(mainOptions.searchAlgos.contains(SearchAlgo::NEIGHBORS_NANOFLANN)) {
                auto neighborsNanoflannKDTree = [&](double radius) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];
                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                        for (size_t i = 0; i < searchSet.numSearches; i++) {
                            std::vector<nanoflann::ResultItem<size_t, double>> ret_matches;
                            const double pt[3] = {points[searchIndexes[i]].getX(), points[searchIndexes[i]].getY(), points[searchIndexes[i]].getZ()};
                            // nanoflann expects squared radius
                            const size_t nMatches = kdtree.template radiusSearch(pt, radius*radius, ret_matches);
                            averageResultSize += nMatches;
                        }
                    averageResultSize /= searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };
                executeBenchmark(neighborsNanoflannKDTree, kernelName, SearchAlgo::NEIGHBORS_NANOFLANN, "none");
            }
        }

        void benchmarkNanoflannKDTreeKNN(NanoFlannKDTree<Container> &kdtree) {
            if(mainOptions.searchAlgos.contains(SearchAlgo::KNN_NANOFLANN)) {
                auto neighborsKNNNanoflann = [&](size_t k) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];
                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                        for (size_t i = 0; i < searchSet.numSearches; i++) {
                            std::vector<size_t> indexes(k);
                            std::vector<double> distances(k);
                            const double pt[3] = {points[searchIndexes[i]].getX(), points[searchIndexes[i]].getY(), points[searchIndexes[i]].getZ()};
                            // nanoflann expects squared radius
                            const size_t nMatches = kdtree.template knnSearch(pt, k, &indexes[0], &distances[0]);
                            averageResultSize += nMatches; // only here so the call is not optimized
                        }
                    averageResultSize /= searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };
                executeKNNBenchmark(neighborsKNNNanoflann, SearchAlgo::KNN_NANOFLANN);
            }
        }

        template <Kernel_t Kernel>
        void benchmarkUnibnOctree(unibn::Octree<Point, Container> &oct, std::string_view kernelName) {
            if(mainOptions.searchAlgos.contains(SearchAlgo::NEIGHBORS_UNIBN)) {
                auto neighborsUnibn = [&](double radius) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];

                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                        for (size_t i = 0; i < searchSet.numSearches; i++) {
                            std::vector<uint32_t> results;
                            if constexpr (Kernel == Kernel_t::sphere) {
                                oct.template radiusNeighbors<unibn::L2Distance<Point>>(points[searchIndexes[i]], radius, results);
                            } else if constexpr (Kernel == Kernel_t::cube) {
                                oct.template radiusNeighbors<unibn::MaxDistance<Point>>(points[searchIndexes[i]], radius, results);
                            } else {
                                static_assert(Kernel == Kernel_t::sphere || Kernel == Kernel_t::cube,
                                            "Unsupported kernel for unibn octree");
                            }
                            averageResultSize += results.size();
                        }

                    averageResultSize /= searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };
                executeBenchmark(neighborsUnibn, kernelName, SearchAlgo::NEIGHBORS_UNIBN, "none");
            }
        }
        
#ifdef HAVE_PCL

        void benchmarkPCLOctreeKNN(pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> &octree, pcl::PointCloud<pcl::PointXYZ> &pclCloud) {
            if(mainOptions.searchAlgos.contains(SearchAlgo::KNN_PCLOCT)) {
                 auto KNN_PCLOCT = [&](size_t k) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];
                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                        for(size_t i = 0; i<searchSet.numSearches; i++) {
                            pcl::PointXYZ searchPoint = pclCloud[searchIndexes[i]];
                            std::vector<int> indexes(k);
                            std::vector<float> distances(k);
                            averageResultSize += octree.nearestKSearch(searchPoint, k, indexes, distances);
                        }
                    averageResultSize = averageResultSize / searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };
                
                executeKNNBenchmark(KNN_PCLOCT, SearchAlgo::KNN_PCLOCT);
            }
        }


        void benchmarkPCLOctree(pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> &octree, pcl::PointCloud<pcl::PointXYZ> &pclCloud, std::string_view kernelName) {
            if(mainOptions.searchAlgos.contains(SearchAlgo::NEIGHBORS_PCLOCT)) {
                auto neighborsPCLOct = [&](double radius) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];
                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                        for(size_t i = 0; i<searchSet.numSearches; i++) {
                            pcl::PointXYZ searchPoint = pclCloud[searchIndexes[i]];
                            std::vector<int> indexes;
                            std::vector<float> distances;
                            averageResultSize += octree.radiusSearch(searchPoint, radius, indexes, distances);
                        }
                    averageResultSize = averageResultSize / searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };
                
                executeBenchmark(neighborsPCLOct, kernelName, SearchAlgo::NEIGHBORS_PCLOCT, "none");
            }
        }

        void benchmarkPCLKDTreeKNN(pcl::KdTreeFLANN<pcl::PointXYZ> &kdtree, pcl::PointCloud<pcl::PointXYZ> &pclCloud) {
            if(mainOptions.searchAlgos.contains(SearchAlgo::KNN_PCLKD)) {
                 auto KNN_PCLKD = [&](size_t k) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];
                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                        for(size_t i = 0; i<searchSet.numSearches; i++) {
                            pcl::PointXYZ searchPoint = pclCloud[searchIndexes[i]];
                            std::vector<int> indexes(k);
                            std::vector<float> distances(k);
                            averageResultSize += kdtree.nearestKSearch(searchPoint, k, indexes, distances);
                        }
                    averageResultSize = averageResultSize / searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };
                
                executeKNNBenchmark(KNN_PCLKD, SearchAlgo::NEIGHBORS_PCLKD);
            }
        }

        void benchmarkPCLKDTree(pcl::KdTreeFLANN<pcl::PointXYZ> &kdtree, pcl::PointCloud<pcl::PointXYZ> &pclCloud, std::string_view kernelName) {
            if(mainOptions.searchAlgos.contains(SearchAlgo::NEIGHBORS_PCLKD)) {
                auto neighborsPCLKD = [&](double radius) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];
                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                        for(size_t i = 0; i<searchSet.numSearches; i++) {
                            pcl::PointXYZ searchPoint = pclCloud[searchIndexes[i]];
                            std::vector<int> indexes;
                            std::vector<float> distances;
                            averageResultSize += kdtree.radiusSearch(searchPoint, radius, indexes, distances);
                        }
                    averageResultSize = averageResultSize / searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };
                
                executeBenchmark(neighborsPCLKD, kernelName, SearchAlgo::NEIGHBORS_PCLKD, "none");
            }
        }

#endif

        template <Kernel_t kernel>
        void benchmarkLinearOctree(LinearOctree<Container>& oct, const std::string_view& kernelName, const OctreeReordered<LinearOctree<Container>, Container>& reordered, ReorderMode mode) {
            typename LinearOctree<Container>::RangeFn getRange = nullptr;
            const std::string_view reorderModeStr = localReorderTypeToString(mode);

            if (mode != ReorderMode::None) {
                getRange = [&](uint32_t leafIndex, const Point& query, double radius) {
                    PrunedRange range = bestRange(leafIndex, query, radius, kernel,
                                                oct, points, reordered, mode, false);
                    const auto& perm = reordered.getLeafPermutation(leafIndex, range.order);
                    return std::make_tuple(&perm, range.iMin, range.iMax);
                };
            }

            if(mainOptions.searchAlgos.contains(SearchAlgo::NEIGHBORS)) {
                auto neighborsSearch = [&](double radius) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];
                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                        for(size_t i = 0; i<searchSet.numSearches; i++) {
                            auto result = oct.template neighbors<kernel>(points[searchIndexes[i]], radius, getRange);
                            averageResultSize += result.size();
                        }
                    averageResultSize /= searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };
                executeBenchmark(neighborsSearch, kernelName, SearchAlgo::NEIGHBORS, reorderModeStr);
            }
            if(mainOptions.searchAlgos.contains(SearchAlgo::NEIGHBORS_PRUNE)) {
                auto neighborsSearchPrune = [&](double radius) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];
                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                        for(size_t i = 0; i<searchSet.numSearches; i++) {
                            auto result = oct.template neighborsPrune<kernel>(points[searchIndexes[i]], radius, getRange);
                            averageResultSize += result.size();
                        }
                    averageResultSize /= searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };
                executeBenchmark(neighborsSearchPrune, kernelName, SearchAlgo::NEIGHBORS_PRUNE, reorderModeStr);
            }
            if(mainOptions.searchAlgos.contains(SearchAlgo::NEIGHBORS_STRUCT)) {
                auto neighborsSearchStruct = [&](double radius) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];
                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                        for(size_t i = 0; i<searchSet.numSearches; i++) {
                            auto result = oct.template neighborsStruct<kernel>(points[searchIndexes[i]], radius, getRange);
                            averageResultSize += result.size();
                        }
                    averageResultSize /= searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };
                executeBenchmark(neighborsSearchStruct, kernelName, SearchAlgo::NEIGHBORS_STRUCT, reorderModeStr);
            }
        }

        void benchmarkLinearOctreeKNN(LinearOctree<Container>& oct) {
            if(mainOptions.searchAlgos.contains(SearchAlgo::KNN_V2)) {
                auto neighborsKNNV2 = [&](size_t k) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];
                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                        for(size_t i = 0; i<searchSet.numSearches; i++) {
                            std::vector<size_t> indexes(k);
                            std::vector<double> distances(k);
                            const size_t nMatches = oct.template knn (points[searchIndexes[i]], k, indexes, distances);
                            averageResultSize += nMatches;
                        }
                    averageResultSize /= searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };
                executeKNNBenchmark(neighborsKNNV2, SearchAlgo::KNN_V2);
            }
        }

        template <Kernel_t kernel>
        void benchmarkPtrOctree(Octree<Container> &oct, std::string_view kernelName, const OctreeReordered<Octree<Container>, Container>& reordered, ReorderMode mode) {
            typename Octree<Container>::RangeFn getRange = nullptr;
            const std::string_view reorderModeStr = localReorderTypeToString(mode);
            if (mode != ReorderMode::None) {
                getRange = [&](uint32_t leafIndex, const Point& query, double radius) {
                    PrunedRange range = bestRange(leafIndex, query, radius, kernel,
                                                oct, points, reordered, mode, false);
                    const auto& perm = reordered.getLeafPermutation(leafIndex, range.order);
                    return std::make_tuple(&perm, range.iMin, range.iMax);
                };
            }

            if(mainOptions.searchAlgos.contains(SearchAlgo::NEIGHBORS_PTR)) {
                auto neighborsPtrSearch = [&](double radius) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];
                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                        for(size_t i = 0; i<searchSet.numSearches; i++) {
                            auto result = oct.template searchNeighbors<kernel>(points[searchIndexes[i]], radius, getRange);
                            averageResultSize += result.size();
                        }
                    averageResultSize /= searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };
                executeBenchmark(neighborsPtrSearch, kernelName, SearchAlgo::NEIGHBORS_PTR, reorderModeStr);
            }
        }
#ifdef HAVE_PICOTREE
        void benchmarkPicoTree(pico_tree::kd_tree<Container> &tree, std::string_view kernelName) {
            if(mainOptions.searchAlgos.contains(SearchAlgo::NEIGHBORS_PICOTREE)) {
                auto neighborsPico = [&](double radius) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];
                    
                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                    for(size_t i = 0; i < searchSet.numSearches; i++) {
                        auto squared_radius = radius * radius;
                        std::vector<typename pico_tree::kd_tree<Container>::neighbor_type> results;
                        tree.search_radius(points[searchIndexes[i]], squared_radius, results);
                        averageResultSize += results.size();
                    }

                    averageResultSize /= searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };
                
                executeBenchmark(neighborsPico, kernelName, SearchAlgo::NEIGHBORS_PICOTREE, "none");
            }
        }

        void benchmarkPicoTreeKNN(pico_tree::kd_tree<Container> &tree) {
            if(mainOptions.searchAlgos.contains(SearchAlgo::KNN_PICOTREE)) {
                auto knnPico = [&](size_t k) -> size_t {
                    size_t averageResultSize = 0;
                    std::vector<size_t> &searchIndexes = searchSet.searchPoints[searchSet.currentRepeat];
                    
                    #pragma omp parallel for schedule(runtime) reduction(+:averageResultSize)
                    for(size_t i = 0; i < searchSet.numSearches; i++) {
                        std::vector<typename pico_tree::kd_tree<Container>::neighbor_type> results;
                        
                        tree.search_knn(points[searchIndexes[i]], k, results);
                        
                        averageResultSize += results.size();
                    }

                    averageResultSize /= searchSet.numSearches;
                    searchSet.nextRepeat();
                    return averageResultSize;
                };

                executeKNNBenchmark(knnPico, SearchAlgo::KNN_PICOTREE);
            }
        }

        void initializeBenchmarkPicoTree() {
            if constexpr (std::is_same_v<Container, PointsAoS>) {
                pico_tree::kd_tree<Container> tree(points, pico_tree::max_leaf_size_t(mainOptions.maxPointsLeaf));
                for (const auto& kernel : mainOptions.kernels) {
                    switch (kernel) {
                        case Kernel_t::sphere:
                            benchmarkPicoTree(tree, kernelToString(kernel));
                            break;
                        default:
                            break; 
                    }
                }
                benchmarkPicoTreeKNN(tree);
            } else {
                std::cout << "WARNING: Skipping pico_tree neighbors benchmarks: Container is not PointsAoS." << std::endl;
            }
        }
#endif
        void initializeBenchmarkNanoflannKDTree() {
            NanoflannPointCloud<Container> npc(points);

            // Build nanoflann kd-tree and run searches
            NanoFlannKDTree<Container> kdtree(3, npc, {mainOptions.maxPointsLeaf});
            for (const auto& kernel : mainOptions.kernels) {
                switch (kernel) {
                    case Kernel_t::sphere:
                        benchmarkNanoflannKDTree(kdtree, kernelToString(kernel));
                        break;
                }
            }
            benchmarkNanoflannKDTreeKNN(kdtree);
        }

        void initializeBenchmarkUnibnOctree() {
            unibn::Octree<Point, Container> oct;
            unibn::OctreeParams params;
            params.bucketSize = mainOptions.maxPointsLeaf;
            oct.initialize(points, params);
            for (const auto& kernel : mainOptions.kernels) {
                switch (kernel) {
                    case Kernel_t::sphere:
                        benchmarkUnibnOctree<Kernel_t::sphere>(oct, kernelToString(kernel));
                        break;
                    case Kernel_t::cube:
                        benchmarkUnibnOctree<Kernel_t::cube>(oct, kernelToString(kernel));
                        break;
                }
            }
        }

#ifdef HAVE_PCL
        void initializeBenchmarkPCLOctree() {
            // Convert cloud to PCL cloud
            auto pclCloud = convertCloudToPCL(points);
            
            // Build the PCL Octree
            pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> oct(mainOptions.pclOctResolution);
            pcl::PointCloud<pcl::PointXYZ>::Ptr cloudPtr = pclCloud.makeShared();
            oct.setInputCloud(cloudPtr);
            oct.addPointsFromInputCloud();;
            
            for (const auto& kernel : mainOptions.kernels) {
                switch (kernel) {
                    case Kernel_t::sphere:
                        benchmarkPCLOctree(oct, pclCloud, kernelToString(kernel));
                        break;
                }
            }
            benchmarkPCLOctreeKNN(oct, pclCloud);
        }
        
        void initializeBenchmarkPCLKDTree() {
            // Convert cloud to PCL cloud
            auto pclCloud = convertCloudToPCL(points);
            
            // Build the PCL Kd-tree
            pcl::KdTreeFLANN<pcl::PointXYZ> kdtree = pcl::KdTreeFLANN<pcl::PointXYZ>();
            pcl::PointCloud<pcl::PointXYZ>::Ptr cloudPtr = pclCloud.makeShared();
            kdtree.setInputCloud(cloudPtr);
            
            for (const auto& kernel : mainOptions.kernels) {
                switch (kernel) {
                    case Kernel_t::sphere:
                        benchmarkPCLKDTree(kdtree, pclCloud, kernelToString(kernel));
                        break;
                }
            }
            benchmarkPCLKDTreeKNN(kdtree, pclCloud);
        }
#endif

            void initializeBenchmarkLinearOctree() {
                // Construcción del octree
                LinearOctree<Container> oct(points, codes, box, enc);
                std::cout << "[LOG] LinearOctree initialized. Num of leaves=" << oct.getNumLeaves() << "\n";

                // Bucle sobre los modos de reordenación
                for (ReorderMode mode : mainOptions.localReorders) {

                    //Declaración del objeto OctreeReordered a pasar por referencia
                    OctreeReordered<std::decay_t<decltype(oct)>, Container> reordered;

                    if (mode != ReorderMode::None) {
                        std::cout << "[LOG] Reordering mode " << (mode == ReorderMode::Cylindrical ? "Cylindrical" : "Spherical") << std::endl;
                        // Measure time taken by reordering to include it in the logs
                        auto startTime = std::chrono::high_resolution_clock::now();
                        reordered.buildLeafPermutations(oct, points, mode);
                        auto endTime = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
                        std::cout << "[LOG] Reordering completed in " << duration << " ms.\n";

                    } else {
                        std::cout << "[LOG] No reordering applied.\n";
                    }
                    
                    // Bucle sobre kernels
                    for (const auto& kernel : mainOptions.kernels) {
                        //debugRangeSelector(oct, reordered, mode, kernel);
                        switch (kernel) {
                            case Kernel_t::sphere:
                                benchmarkLinearOctree<Kernel_t::sphere>(oct, kernelToString(kernel), reordered, mode);
                                break;
                            case Kernel_t::circle:
                                benchmarkLinearOctree<Kernel_t::circle>(oct, kernelToString(kernel), reordered, mode);
                                break;
                            case Kernel_t::cube:
                                benchmarkLinearOctree<Kernel_t::cube>(oct, kernelToString(kernel), reordered, mode);
                                break;
                            case Kernel_t::square:
                                benchmarkLinearOctree<Kernel_t::square>(oct, kernelToString(kernel), reordered, mode);
                                break;
                        }
                    }
                }

                // Benchmark kNN (si aplica)
                benchmarkLinearOctreeKNN(oct);

                std::cout << "[LOG] Finished benchmarks for this reordering mode.\n";
            }

        void initializeBenchmarkPtrOctree() {
            Octree oct(points, box);
            std::cout << "[LOG] PtrOctree initialized. Num of leaves=" << oct.getNumLeaves() << "\n";

            // Bucle sobre los modos de reordenación
            for (ReorderMode mode : mainOptions.localReorders) {

                //Declaración del objeto OctreeReordered a pasar por referencia
                
                OctreeReordered<std::decay_t<decltype(oct)>, Container> reordered;


                if (mode != ReorderMode::None) {
                    std::cout << "[LOG] Reordering mode " << (mode == ReorderMode::Cylindrical ? "Cylindrical" : "Spherical") << std::endl;
                    // Measure time taken by reordering to include it in the logs
                    auto startTime = std::chrono::high_resolution_clock::now();
                    reordered.buildLeafPermutations(oct, points, mode);
                    auto endTime = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
                    std::cout << "[LOG] OctreeReordered object initialized in " << duration << " ms.\n";
                } else {
                    std::cout << "[LOG] No reordering applied.\n";
                }

                for (const auto& kernel : mainOptions.kernels) {
                    switch (kernel) {
                        case Kernel_t::sphere:
                            benchmarkPtrOctree<Kernel_t::sphere>(oct, kernelToString(kernel), reordered, mode);
                            break;
                        case Kernel_t::circle:
                            benchmarkPtrOctree<Kernel_t::circle>(oct, kernelToString(kernel), reordered, mode);
                            break;
                        case Kernel_t::cube:
                            benchmarkPtrOctree<Kernel_t::cube>(oct, kernelToString(kernel), reordered, mode);
                            break;
                        case Kernel_t::square:
                            benchmarkPtrOctree<Kernel_t::square>(oct, kernelToString(kernel), reordered, mode);
                            break;
                    }
                }
                std::cout << "[LOG] Finished benchmarks for this reordering mode.\n";
            }
        }


        /// @brief Main benchmarking function
        void runAllBenchmarks() {
            printBenchmarkInfo();
            int currentStructureBenchmark = 1;
            int totalStructureBenchmarks = mainOptions.searchStructures.size();
            for(SearchStructure structure: mainOptions.searchStructures) {
                std::cout << "Starting benchmarks over structure " << searchStructureToString(structure) 
                    << " (" << currentStructureBenchmark << " out of " << totalStructureBenchmarks << " structures)" << std::endl; 
                switch(structure) {
                    case SearchStructure::PTR_OCTREE:
                        initializeBenchmarkPtrOctree();
                    break;
                    case SearchStructure::LINEAR_OCTREE:
                        if(!enc.is3D()) {
                            std::cout << "Skipping Linear Octree since reordering is not 3D!" << std::endl;
                        } else {
                            initializeBenchmarkLinearOctree();
                        }
                    break;
#ifdef HAVE_PCL
                    case SearchStructure::PCL_KDTREE:
                        initializeBenchmarkPCLKDTree();
                    break;
                    case SearchStructure::PCL_OCTREE:
                        initializeBenchmarkPCLOctree();
                    break;
#endif
                    case SearchStructure::UNIBN_OCTREE:
                        initializeBenchmarkUnibnOctree();
                    break;
                    case SearchStructure::NANOFLANN_KDTREE:
                        initializeBenchmarkNanoflannKDTree();
                    break;
#ifdef HAVE_PICOTREE
                    case SearchStructure::PICOTREE:
                        initializeBenchmarkPicoTree();
                    break;
#endif
                    default:
                        std::cerr << "Unknown SearchStructure type!" << std::endl;
                    break;
                }
                currentStructureBenchmark++;
            }
        }

        SearchSet& getSearchSet() const { return searchSet; }
};
