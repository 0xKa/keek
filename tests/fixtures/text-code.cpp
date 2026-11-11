#include <iostream>
#include <string>
#include <vector>

struct Preview {
    std::string file;
    int width = 960;
    int height = 640;
};

int main()
{
    const std::vector<Preview> previews{{"a.png"}, {"b.txt", 800, 600}};
    for (const Preview &preview : previews) {
        std::cout << preview.file << ": " << preview.width << 'x' << preview.height << '\n';
    }
    return 0;
}
