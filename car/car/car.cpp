#include <iostream>       // Для ввода/вывода в консоль (std::cout, std::cerr)
#include <fstream>        // Для работы с файлами (чтение/запись)
#include <string>         // Для работы со строками (std::string)
#include <vector>         // Для использования динамических массивов (std::vector)
#include <regex>          // Для работы с регулярными выражениями (поиск и замена текста)
#include <filesystem>     // Для работы с файловой системой (копирование файлов)

namespace fs = std::filesystem;  // Сокращение для удобства использования std::filesystem

// Функция для парсинга файла с ошибками
std::vector<std::pair<int, std::string>> parse_error_file(const std::string& error_file) {
    std::vector<std::pair<int, std::string>> corrections;  // Вектор для хранения пар (номер строки, новый ID провинции)
    std::ifstream file(error_file);  // Открываем файл с ошибками для чтения
    std::string line;  // Переменная для хранения текущей строки файла
    std::regex error_regex(R"(error line (\d+).*but was (\d+))");  // Регулярное выражение для поиска номеров строк и ID провинций

    while (std::getline(file, line)) {  // Читаем файл построчно
        std::smatch matches;  // Хранит результаты поиска по регулярному выражению
        if (std::regex_search(line, matches, error_regex)) {  // Ищем совпадения в строке
            if (matches.size() == 3) {  // Проверяем, что найдено 2 группы (номер строки и ID провинции)
                int line_num = std::stoi(matches[1]);  // Преобразуем номер строки в число
                std::string new_id = matches[2];  // Получаем новый ID провинции
                corrections.emplace_back(line_num, new_id);  // Добавляем пару в вектор
            }
        }
    }

    return corrections;  // Возвращаем вектор с исправлениями
}

// Функция для обработки файла buildings.txt
void process_buildings_file(const std::string& buildings_file,
    const std::vector<std::pair<int, std::string>>& corrections) {
    // Создаем резервную копию файла перед изменениями
    fs::copy(buildings_file, buildings_file + ".bak", fs::copy_options::overwrite_existing);

    // Читаем весь файл в память
    std::ifstream in_file(buildings_file);  // Открываем файл для чтения
    std::vector<std::string> lines;  // Вектор для хранения всех строк файла
    std::string line;  // Переменная для текущей строки

    while (std::getline(in_file, line)) {  // Читаем файл построчно
        lines.push_back(line);  // Добавляем строку в вектор
    }
    in_file.close();  // Закрываем файл

    // Применяем изменения к строкам
    bool modified = false;  // Флаг, были ли внесены изменения
    for (const auto& [line_num, new_id] : corrections) {  // Перебираем все исправления
        if (line_num > 0 && line_num <= lines.size()) {  // Проверяем, что номер строки корректен
            std::string& current_line = lines[line_num - 1];  // Получаем ссылку на строку (индексация с 0)
            size_t pos = current_line.find(';');  // Ищем позицию первого разделителя ';'
            if (pos != std::string::npos) {  // Если разделитель найден
                current_line.replace(0, pos, new_id);  // Заменяем ID провинции (от начала до первого ';')
                modified = true;  // Устанавливаем флаг изменений
            }
        }
    }

    // Записываем изменения обратно в файл
    if (modified) {
        std::ofstream out_file(buildings_file);  // Открываем файл для записи
        for (const auto& l : lines) {  // Перебираем все строки
            out_file << l << "\n";  // Записываем строку в файл
        }
        std::cout << "Успешно исправлено " << corrections.size() << " строк.\n";  // Выводим отчет
    }
    else {
        std::cout << "Изменения не требуются.\n";  // Сообщаем, если изменений не было
    }
}

int main() {
    system("chcp 65001 > nul");  // Переключает кодировку консоли на UTF-8 (для корректного вывода кириллицы)
    setlocale(LC_ALL, "russian");  // Устанавливает локаль для поддержки русского языка

    // Указываем пути к файлам (можно заменить на пользовательский ввод)
    std::string buildings_path = R"(buildings.txt)";  // Файл с данными построек
    std::string errors_path = R"(er.txt)";  // Файл с ошибками

    try {
        auto corrections = parse_error_file(errors_path);  // Парсим файл с ошибками
        if (corrections.empty()) {  // Если нет ошибок
            std::cout << "Не найдено ошибок для исправления.\n";
            return 0;  // Завершаем программу
        }

        process_buildings_file(buildings_path, corrections);  // Обрабатываем основной файл
    }
    catch (const std::exception& e) {  // Ловим исключения
        std::cerr << "Ошибка: " << e.what() << "\n";  // Выводим сообщение об ошибке
        return 1;  // Возвращаем код ошибки
    }

    return 0;  // Успешное завершение программы
}