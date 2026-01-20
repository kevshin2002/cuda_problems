#ifndef PROBLEM_RUNNER_H
#define PROBLEM_RUNNER_H

#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class ProblemRunner {
public:
    using Line = std::vector<int>;
    using Lines = std::vector<Line>;

    template <typename KernelFunctor>
    static int Run(const std::string& input_path,
                   const std::string& solution_path,
                   const KernelFunctor& functor) {
        Lines inputs = ReadLines(input_path);
        Lines solutions = ReadLines(solution_path);

        size_t total = inputs.size();
        size_t passed = 0;
        std::vector<size_t> failed_indices;

        size_t max_lines = inputs.size();
        if (solutions.size() < max_lines) {
            max_lines = solutions.size();
        }

        for (size_t i = 0; i < max_lines; ++i) {
            Line output;
            functor(inputs[i], output);
            if (CompareLine(output, solutions[i])) {
                ++passed;
            } else {
                failed_indices.push_back(i);
            }
        }

        if (inputs.size() != solutions.size()) {
            for (size_t i = max_lines; i < total; ++i) {
                failed_indices.push_back(i);
            }
        }

        const char* color_green = "\x1b[32m";
        const char* color_red = "\x1b[31m";
        const char* color_yellow = "\x1b[33m";
        const char* color_blue = "\x1b[34m";
        const char* color_cyan = "\x1b[36m";
        const char* color_reset = "\x1b[0m";

        for (size_t i = 0; i < max_lines; ++i) {
            Line output;
            functor(inputs[i], output);
            bool ok = CompareLine(output, solutions[i]);
            printf("%sline %zu:%s %s%s%s\n",
                   color_blue, i, color_reset,
                   ok ? color_green : color_red,
                   ok ? "PASSED" : "FAILED",
                   color_reset);
            printf("  %sactual:%s", color_yellow, color_reset);
            PrintLine(output);
            printf("  %sexpected:%s", color_yellow, color_reset);
            PrintLine(solutions[i]);
        }

        if (inputs.size() != solutions.size()) {
            printf("%swarning:%s input and solution line counts differ\n",
                   color_yellow, color_reset);
        }

        if (!failed_indices.empty()) {
            printf("%sfailed lines%s (0-based):", color_red, color_reset);
            for (size_t idx : failed_indices) {
                printf(" %zu", idx);
            }
            printf("\n");
        }

        printf("%stotal:%s %zu, %spassed:%s %zu, %sfailed:%s %zu\n",
               color_cyan, color_reset, total,
               color_green, color_reset, passed,
               color_red, color_reset, failed_indices.size());

        return failed_indices.empty() ? 0 : 1;
    }

private:
    static Lines ReadLines(const std::string& path) {
        Lines lines;
        std::ifstream file(path);
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            Line values;
            int value = 0;
            while (iss >> value) {
                values.push_back(value);
            }
            if (!values.empty()) {
                lines.push_back(values);
            }
        }
        return lines;
    }

    static bool CompareLine(const Line& actual, const Line& expected) {
        if (actual.size() != expected.size()) {
            return false;
        }
        for (size_t i = 0; i < actual.size(); ++i) {
            if (actual[i] != expected[i]) {
                return false;
            }
        }
        return true;
    }

    static void PrintLine(const Line& line) {
        if (line.empty()) {
            printf(" (empty)\n");
            return;
        }
        printf(" ");
        for (size_t i = 0; i < line.size(); ++i) {
            if (i > 0) {
                printf(" ");
            }
            printf("%d", line[i]);
        }
        printf("\n");
    }
};

#endif
