#include <iostream>
#include <cstdlib>
#include <utility>
#include "vector"
#include "fstream"
#include "functional"
#include "./includes/common.h"

// 自定义的内存分配函数
void* operator new(std::size_t size) {
    std::cout << "Allocating " << size << " bytes" << std::endl;
    void* p = malloc(size);
    if (!p) {
        throw std::bad_alloc();
    }
    return p;
}

void operator delete(void* memory) {
    free(memory);
}

void* operator new[](std::size_t size) {
    std::cout << "Allocating array of " << size << " bytes" << std::endl;
    void* p = malloc(size);
    if (!p) {
        throw std::bad_alloc();
    }
    return p;
}

void operator delete[](void* memory) {
    std::cout << "Deallocating array of " << memory << std::endl;
    free(memory);
}

class ScopeGuard {
private:
    typedef std::function<void()> destructor_type;

    destructor_type destructor_;
    bool dismissed_;

public:
    ScopeGuard(destructor_type destructor) : destructor_(std::move(destructor)), dismissed_(false) {}

    ~ScopeGuard()
    {
        if (!dismissed_) {
            destructor_();
        }
    }

    void dismiss() { dismissed_ = true; }

    ScopeGuard(ScopeGuard const&) = delete;
    ScopeGuard& operator=(ScopeGuard const&) = delete;
};

void scopeguard_func() {
    std::ofstream file;
    file.open("./wow.txt");
    ScopeGuard guard{[&file]() {std::cout<<"file closed"<<std::endl; file.close();}};
}

class content_obj {
public:
//    content_obj(): content_obj_content(nullptr) {}
    explicit content_obj(std::string content): content_obj_content(std::move(content)) {
        std::cout << "content_obj construction with string" << std::endl;
    }
    content_obj(const content_obj& val) {
        std::cout << "content_obj construction copied" << std::endl;
        content_obj_content = val.content_obj_content;
    }
    content_obj(content_obj&& val) noexcept {
        std::cout << "content_obj construction moved" << std::endl;
        content_obj_content = std::move(val.content_obj_content);
    }
    content_obj& operator=(const content_obj& rvalue) {
        std::cout << "content_obj = rvalue(copy triggered)" << std::endl;
        if (this != &rvalue) {
            content_obj_content = rvalue.content_obj_content;
        }
        return *this;
    }
    content_obj& operator=(content_obj&& rvalue) noexcept {
        std::cout << "content_obj = rvalue(move triggered)" << std::endl;
        if (this != &rvalue) {
            content_obj_content = rvalue.content_obj_content;
        }
        return *this;
    }
    friend inline std::ostream& operator << (std::ostream& out, const content_obj& obj) {
        out << obj.content_obj_content;
        return out;
    }
private:
    std::string content_obj_content;
};

class my_obj {
public:
    my_obj(): obj_content("") {}
    // move constructor
    explicit my_obj(content_obj obj_content): obj_content(std::move(obj_content)) {}
    // not-very-well implementation of move constructor
//    explicit my_obj(const content_obj& obj_content): obj_content(obj_content) {}

    void print_me() const {
        std::cout << obj_content << std::endl;
    }

private:
    content_obj obj_content;
};

int main() {
    unit_test::vec3_test();
    std::cout << "-------------------------------------" << std::endl;
    int* myInt = new int[3];
    delete[] myInt;
    int k = 10, m = 20;
    k += (m += 9);  // += will return the value of result
    std::cout << k << std::endl;
    const int &valid = 10;
    std::cout << valid << std::endl;
//    int &invalid = 10;
//    unit_test::vec3_test();
//    unit_test::ray_test();
    std::cout << normalize(vec3{1.7, 1.0, 1}) << std::endl;
    const int* const ptr_k = &k;
//    *ptr_k = 10;
//    ptr_k = &m;
    scopeguard_func();
    std::vector<int> vec;

    std::cout << "-------------------------------------" << std::endl;
    /*
     * if we use the move constructor:
        Allocating 16 bytes
        Allocating 32 bytes
        Allocating 16 bytes
        content_obj construction with string
        Allocating 16 bytes
        content_obj construction moved
        109231bkjbdkhasvdauiidfo
     */
    /*
     * if we use the not-very-well implemented move constructor
        Allocating 16 bytes
        Allocating 32 bytes
        Allocating 16 bytes
        content_obj construction with string
        Allocating 16 bytes
        content_obj construction copied
        Allocating 32 bytes
        109231bkjbdkhasvdauiidfo
     */
    my_obj obj(content_obj("109231bkjbdkhasvdauiidfo"));
    obj.print_me();


    return 0;
}