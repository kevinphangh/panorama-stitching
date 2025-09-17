#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H

#include <string>
#include <vector>
#include <fstream>
#include "experiment_runner.h"

class ReportGenerator {
public:
    ReportGenerator() = default;

    // Generate markdown report from experiment results
    bool generateMarkdownReport(const std::vector<ExperimentResult>& results,
                               const std::string& output_path);

    // Export results to CSV format
    bool exportToCSV(const std::vector<ExperimentResult>& results,
                    const std::string& csv_path);

    // Export match distances to separate CSV files
    bool exportMatchDistances(const std::vector<ExperimentResult>& results,
                            const std::string& output_dir);

    // Save individual experiment results to files
    bool saveExperimentResults(const std::vector<ExperimentResult>& results,
                              const std::string& output_dir);

private:
    // Helper functions
    std::string formatDuration(double ms) const;
    std::string generateSummaryTable(const std::vector<ExperimentResult>& results) const;
    std::string generateDetailedResults(const std::vector<ExperimentResult>& results) const;

    // CSV helpers
    void writeCSVHeader(std::ofstream& file) const;
    void writeCSVRow(std::ofstream& file, const ExperimentResult& result) const;
};

#endif // REPORT_GENERATOR_H