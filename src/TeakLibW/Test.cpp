#include "StdAfx.h"

#include <iostream>

static int errors = 0;

class TestElement {
  public:
    TestElement() = default;
    TestElement(int i) {
        NumberA = i;
        Name = bprintf("SomeID %d END", i);
    }
    bool operator==(const TestElement &rhs) const { return Name == rhs.Name && NumberA == rhs.NumberA && Sinn == rhs.Sinn; }
    bool operator!=(const TestElement &rhs) const { return !operator==(rhs); }

    friend bool operator<(const TestElement &lhs, const TestElement &rhs) { return lhs.NumberA < rhs.NumberA; }

    friend std::ostream &operator<<(std::ostream &stream, const TestElement &i) {
        stream << i.Name << ";" << i.NumberA;
        return stream;
    }

  private:
    CString Name;
    int NumberA{};
    int Sinn{42};
};

template <typename T> void expect_func(ALBUM_V<T> &list, const std::vector<T> &target, int size, int free, int txt) {
    list.check_consistent_index();
    if (list != target) {
        ++errors;
        std::cout << "Test on line " << txt << " FAILED. List has wrong content (expectation vs. reality):" << std::endl;
        for (auto &i : target) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        for (auto &i : list) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }
    if (list.AnzEntries() != size || list.GetNumFree() != free || list.GetNumUsed() != (size - free)) {
        ++errors;
        std::cout << "Test on line " << txt << " FAILED. Length incorrect." << std::endl;
    }
}
#define expect_list(l, t, s, f) expect_func(l, t, s, f, __LINE__)

template <typename T> void expect_val_func(ALBUM_V<T> &list, int id, int target, int txt) {
    list.check_consistent_index();
    auto val = list[id];
    auto t = T(target);
    if (val != t) {
        ++errors;
        std::cout << "Test on line " << txt << " FAILED. Expected '" << t << "' got '" << val << "' for ID = " << id << std::endl;
    }
}
#define expect_val(l, i, e) expect_val_func(l, i, e, __LINE__)

template <typename T> void expect_nonexist_func(ALBUM_V<T> &list, int id, int txt) {
    list.check_consistent_index();
    if (list.IsInAlbum(id)) {
        std::cout << "Test on line " << txt << " FAILED. Element with ID = " << id << " should not exist, but IsInAlbum() returned true" << std::endl;
        ++errors;
    }

    bool exception = false;
    try {
        list[id] = T(99);
    } catch (std::runtime_error e) {
        exception = true;
    }
    if (!exception) {
        std::cout << "Test on line " << txt << " FAILED. Element with ID = " << id << " should not exist, but operator[] did not raise exception" << std::endl;
        ++errors;
    }
}
#define expect_nonexist(l, i) expect_nonexist_func(l, i, __LINE__)

template <typename T> bool run_test() {
    ALBUM_V<T> list("Test");

    /* test basic function */
    list.ReSize(4);
    list += 1;
    list += 2;
    auto id = (list += 3);
    list += 4;
    expect_list(list, std::vector<T>({4, 3, 2, 1}), 4, 0);
    expect_val(list, id, 3);

    /* test resizing */
    list.ReSize(6);
    auto id2 = (list += 5);
    expect_list(list, std::vector<T>({4, 3, 2, 1, T(), 5}), 6, 1);
    expect_val(list, id, 3);
    expect_val(list, id2, 5);

    /* test copy + shrinking */
    {
        auto tmp = list;
        tmp.ReSize(3);
        expect_val(tmp, id, 3);
        tmp[id] = 99;
        expect_val(tmp, id, 99);
        expect_list(tmp, std::vector<T>({4, 99, 2}), 3, 0);

        /* deleted element */
        expect_nonexist(tmp, id2);

        /* test original model unchanged */
        expect_list(list, std::vector<T>({4, 3, 2, 1, T(), 5}), 6, 1);
    }

    /* test removal */
    {
        auto tmp = list;
        expect_val(tmp, id, 3);
        expect_val(tmp, id2, 5);
        tmp -= id2;
        expect_val(tmp, id, 3);
        expect_list(tmp, std::vector<T>({4, 3, 2, 1, T(), 5}), 6, 2);
        expect_nonexist(tmp, id2);
    }

    /* test sorting */
    {
        auto tmp = list;
        expect_val(list, id, 3);
        list.Sort();
        expect_val(list, id, 3);
        expect_val(list, id2, 5);
        expect_list(list, std::vector<T>({1, 2, 3, 4, 5, T()}), 6, 1);

        tmp -= id2;
        expect_val(tmp, id, 3);
        tmp.Sort();
        expect_val(tmp, id, 3);
        expect_nonexist(tmp, id2);
        expect_list(tmp, std::vector<T>({1, 2, 3, 4, T(), 5}), 6, 2);
    }

    printf("Errors: %d \n", errors);
    return (errors == 0);
}

bool run_regression() {
    run_test<int>();
    run_test<TestElement>();
    return (errors == 0);
}
