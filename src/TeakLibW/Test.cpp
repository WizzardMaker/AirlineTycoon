#include "StdAfx.h"

#include <iostream>

#define AT_Log(...) AT_Log_I("Error", __VA_ARGS__)

static SLONG errors = 0;

class TestElement {
  public:
    TestElement() = default;
    TestElement(SLONG i) {
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
    SLONG NumberA{};
    SLONG Sinn{42};
};

template <typename T> void expect_func(ALBUM_V<T> &list, const std::vector<T> &target, SLONG size, SLONG free, SLONG txt) {
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

template <typename T> void expect_val_func(ALBUM_V<T> &list, SLONG id, SLONG target, SLONG txt) {
    list.check_consistent_index();
    auto val = list[id];
    auto t = T(target);
    if (val != t) {
        ++errors;
        std::cout << "Test on line " << txt << " FAILED. Expected '" << t << "' got '" << val << "' for ID = " << id << std::endl;
    }
}
#define expect_val(l, i, e) expect_val_func(l, i, e, __LINE__)

template <typename T> void expect_nonexist_func(ALBUM_V<T> &list, SLONG id, SLONG txt) {
    list.check_consistent_index();
    if (list.IsInAlbum(id)) {
        std::cout << "Test on line " << txt << " FAILED. Element with ID = " << id << " should not exist, but IsInAlbum() returned true" << std::endl;
        ++errors;
    }

    bool exception = false;
    try {
        list[id] = T(99);
    } catch (const std::runtime_error& e) {
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
        expect_val(tmp, id, 3);
        tmp.Sort();
        expect_val(tmp, id, 3);
        expect_val(tmp, id2, 5);
        expect_list(tmp, std::vector<T>({1, 2, 3, 4, 5, T()}), 6, 1);

        tmp -= id;
        expect_nonexist(tmp, id);
        expect_val(tmp, id2, 5);
        tmp.Sort();
        expect_nonexist(tmp, id);
        expect_val(tmp, id2, 5);
        expect_list(tmp, std::vector<T>({1, 2, 4, 5, 3, T()}), 6, 2);
    }

    /* test push front */
    list.ReSize(10);
    list *= 11;
    auto id3 = (list *= 12);
    expect_list(list, std::vector<T>({4, 3, 2, 1, 11, 5, 12, T(), T(), T()}), 10, 3);
    expect_val(list, id, 3);
    expect_val(list, id2, 5);
    expect_val(list, id3, 12);

    /* insert using ID */
    list.ReSize(12);
    list.push_back(0x2000004, 20);
    list.push_back(0x2000003, 21);
    list.push_front(0x2000002, 22);
    list.push_front(0x2000001, 23);
    expect_list(list, std::vector<T>({4, 3, 2, 1, 11, 5, 12, 22, 23, T(), 21, 20}), 12, 1);
    expect_val(list, 0x2000004, 20);
    expect_val(list, 0x2000003, 21);
    expect_val(list, 0x2000002, 22);
    expect_val(list, 0x2000001, 23);

    /* test swapping by id */
    list.Swap(id, id2);
    list.Swap(id2, id3);
    expect_val(list, id, 3);
    expect_val(list, id2, 5);
    expect_val(list, id3, 12);
    expect_list(list, std::vector<T>({4, 12, 2, 1, 11, 3, 5, 22, 23, T(), 21, 20}), 12, 1);

    /* test swapping by index */
    list.Swap(0, 1);
    list.Swap(0, 4);
    expect_val(list, id, 3);
    expect_val(list, id2, 5);
    expect_val(list, id3, 12);
    expect_list(list, std::vector<T>({11, 4, 2, 1, 12, 3, 5, 22, 23, T(), 21, 20}), 12, 1);

    AT_Log("Errors: %d", errors);
    return (errors == 0);
}

bool run_regression() {
    run_test<SLONG>();
    run_test<TestElement>();
    return (errors == 0);
}
