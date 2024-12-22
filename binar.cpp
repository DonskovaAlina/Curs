#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main() {
    // Создаем векторы для записи в файл
    vector<vector<float>> vectors = {
        {1.0f, 2.0f, 3.0f}, // Вектор 1
        {4.0f, 5.0f}        // Вектор 2
    };

    // Открываем файл для записи в бинарном формате
    ofstream file("input_vectors.bin", ios::binary);
    
    if (!file) {
        cerr << "Ошибка открытия файла для записи." << endl;
        return 1;
    }

    // Записываем количество векторов в файл
    uint32_t N = vectors.size();
    file.write(reinterpret_cast<const char*>(&N), sizeof(N));

    // Записываем каждый вектор в файл
    for (const auto& vec : vectors) {
        uint32_t S = vec.size();
        file.write(reinterpret_cast<const char*>(&S), sizeof(S)); // Записываем размер вектора
        file.write(reinterpret_cast<const char*>(vec.data()), S * sizeof(float)); // Записываем элементы вектора
    }

    file.close();
    cout << "Файл успешно создан." << endl;

    return 0;
}