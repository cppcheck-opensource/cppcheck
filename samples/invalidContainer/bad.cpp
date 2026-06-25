#include <vector>
int main()
{
    std::vector<int> items;
    items.push_back(1);
    items.push_back(2);
    items.push_back(3);
    std::vector<int>::iterator iter;
    for (iter = items.begin(); iter != items.end(); ++iter) {
        if (*iter == 2) {
            items.erase(iter);
        }
    }
}
