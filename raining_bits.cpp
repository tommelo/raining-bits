#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>
#include <thread>
#include <sys/ioctl.h>
#include <random>
#include <unistd.h>
#include <term.h>

const std::string RESET = "\033[0m";
const std::string BOLD = "\033[1m";
const std::string CLEAR_SCREEN = "\033[2J\033[1;1H";

const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";
const std::vector<std::string> COLORS = {RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};

const std::vector<std::string> RAIN_BITS {"0", "1"};
const std::vector<int> RAIN_DROP_FREQUENCY {50, 60, 70, 80, 90};
const int PRINT_MATRIX_FREQUENCY = 60;
const int FILL_MATRIX_FREQUENCY = 500;

std::random_device random_device;
std::mt19937 random_engine(random_device());

template<class T>
T random_element(int low, int high, std::vector<T> list)
{
    std::uniform_int_distribution<> distribution(low, high);
    return list[distribution(random_engine)];
}

void fill_matrix(std::vector<std::vector<std::string>> &matrix, std::vector<int> random_start) 
{
    int rows = matrix.size();
    int cols = matrix[0].size();

    while (true)
    {
        for (int i = 0; i < rows - 1; i++)
        {
            for (int j = 0; j < cols - 1; j++)
            {
                if (i == 0 && j % 7 == 0)
                {
                    int random_row_index = random_element(0, random_start.size() - 1, random_start);
                    std::string color = random_element(0, COLORS.size() - 1, COLORS);
                    std::string bit = random_element<std::string>(0, RAIN_BITS.size() - 1, RAIN_BITS);
                    matrix[random_row_index][j] = BOLD + color + bit + RESET;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(FILL_MATRIX_FREQUENCY));   
    }
}

void move(std::vector<std::vector<std::string>> &matrix, int col, int frequency)
{
    while (true)
    {
        for(int i = matrix.size() - 1; i >= 1; i--)
        {
            matrix[i][col] = matrix[i - 1][col];
            matrix[i - 1][col] = "";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(frequency));
    }    
}


int main(int argc, char *argv[])
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    int rows = static_cast<int>(w.ws_row);
    int cols = static_cast<int>(w.ws_col);
    int half_of_rows = std::floor(rows / 2);

    std::vector<std::vector<std::string>> matrix(rows, std::vector<std::string>(cols));
    std::vector<int> random_start(half_of_rows);
    for (int i = 0; i < half_of_rows; i++)
    {
        random_start.push_back(i);
    }

    std::thread fill_matrix_thread([&matrix, random_start](){fill_matrix(matrix, random_start);});
    fill_matrix_thread.detach();
    
    std::vector<std::thread> threads;
    for (int j = 0; j < cols; j++) 
    {
        int frequency = random_element(0, RAIN_DROP_FREQUENCY.size() - 1, RAIN_DROP_FREQUENCY);
        threads.emplace_back([&matrix, j, frequency](){ move(matrix, j, frequency); });
    }

    while (true)
    {
        for (int i = 0; i < rows - 1; i++)
        {
            for (int j = 0; j < cols - 1; j++)
            {
                std::cout << std::setw(1) << matrix[i][j];                
            }
            std::cout << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(PRINT_MATRIX_FREQUENCY));
        std::cout << CLEAR_SCREEN;
    }
    
    return EXIT_SUCCESS;
}