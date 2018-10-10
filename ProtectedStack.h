#include <iostream>
#include <functional>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <inttypes.h>
#include <unordered_set>
#include "Colors.h"


#ifdef private
#undef private
#endif 

#ifndef debug
#define debug false
#endif


struct StackHelperForCopyError {
  
  static std::unordered_set<const void*> array_of_stack_starts_;
};

std::unordered_set<const void*> StackHelperForCopyError::array_of_stack_starts_;


template <class T>
class ProtectedStack : public StackHelperForCopyError {
 public:

  static constexpr int MAX_STACK_SIZE_IN_BYTES_ = 1e7;
  static constexpr size_t MIN_NUM_OF_ELEMENTS_  = 4;
  static constexpr size_t SIZE_OF_TYPE_         = sizeof(T);

  //for debug
  static constexpr int MAGIC_NUM_FOR_CHECKER_SUM_   = 2689;
  static constexpr int MAGIC_NUM_FOR_FIRST_CHECKER_ = 769831;
  StackHelperForCopyError helper_;

  size_t num_of_blocks_in_memory_;
  size_t curr_max_size_                   = 0;
  size_t size_                            = 0;
  char* array_                            = nullptr;
  char* copy_start_stack_pointer_         = nullptr;
  size_t first_checker_                   = MAGIC_NUM_FOR_FIRST_CHECKER_;
  std::hash<T> hash_function_;
  static constexpr size_t START_OF_ARRAY_ = debug ? sizeof(void*) : 0;

  
  ////////////////////////////////////////////////////
  
  size_t GetIndexInStackArray(size_t i) const {
    
    return START_OF_ARRAY_ + i * SIZE_OF_TYPE_; 
  }

  
  size_t GetLastIndex() const {

    return GetIndexInStackArray(size_);
  }


  size_t GetCanaryEndIndex() const {
    
    return num_of_blocks_in_memory_ - sizeof(void*);
  }
  

  constexpr size_t GetCanaryStartIndex() const {
    
    return 0;
  }


  size_t GetCheckSumIndex() const {
    
    return num_of_blocks_in_memory_ - sizeof(void*) - sizeof(int);
  }


  const void* CHECKER_THIS() const {
  
    return this;
  }


  void* CHECKER_CANARY_END_() const {
     
    return reinterpret_cast<void*>(array_);
  }


  const void* CHECKER_CANARY_START_() const {
     
    return this;
  }


  const void* canary_end_() const {
    
    return *reinterpret_cast<void**>(array_ + GetCanaryEndIndex());
  }

  int int_canary_end_() const {
    return *reinterpret_cast<int*>(array_ + GetCanaryEndIndex());
  }
  
    
  const void* canary_start_() const {
    
    return *reinterpret_cast<void**>(array_ + GetCanaryStartIndex());
  }


  void CreateEndCanary() {
    
    *reinterpret_cast<void**>(array_ + GetCanaryEndIndex()) = CHECKER_CANARY_END_();
  }
  
  void CreateStartCanary() {
    
    *reinterpret_cast<const void**>(array_ + GetCanaryStartIndex()) = 
                                    CHECKER_CANARY_START_();
  }

  //////////////////////////////////////////////////////


  size_t GetSizeInBytes(int num_of_elements) const {
    
    return SIZE_OF_TYPE_ * num_of_elements + 
           (debug ? sizeof(int) + 2 * sizeof(void*) : 0); 
  }


  int& GetCheckSum() {

    return *reinterpret_cast<int*>(array_ + GetCheckSumIndex());
  }


  int GetCheckSum() const {

    return *reinterpret_cast<int*>(array_ + GetCheckSumIndex());
  }


  int GetCurrCheckSum() const {
    
    int ans = 0;
    for (int i = 0; i < size_; ++i) {
      ans ^= hash_function_(GetI(i)) * (i + 1);
    }
    ans   ^= MAGIC_NUM_FOR_CHECKER_SUM_;
    ans   ^= size_;
    return ans;
  }


  size_t GetEndOFArrayElements() const {
    
    int additional_memory = debug ? sizeof(void*) + 
                                    sizeof(int): 0;
    return num_of_blocks_in_memory_ - additional_memory;
  }


  size_t GetCurrAllStackSize() const {
    
    return (GetEndOFArrayElements() - sizeof(void*)) / SIZE_OF_TYPE_;
  }


  const T& GetI(size_t i) const {
     
    return *reinterpret_cast<T*>(array_ + GetIndexInStackArray(i));
  }


  T& GetI(size_t i) {
  
    return *reinterpret_cast<T*>(array_ + GetIndexInStackArray(i));
  }
  

  void DestructI(size_t i) {
    
    reinterpret_cast<T*>(array_ + GetIndexInStackArray(i))->~T();

  }
  //////////////////////////////////////////////////////

  
  bool StringIsCorrectForDump(bool checker) const {
    if (checker) {
     
      std::cout << green << "   (OK)" << def;
      return true;
    } else {
      
      std::cout << red << "   (ERROR!)" << def;
      return false;
    }
  }

  
  bool IsCorrectThisPointer() const {
    
    return this != nullptr;
  }

  
  bool IsCorrectArrayPointer() const {
    
    return array_ != nullptr &&
           copy_start_stack_pointer_ == array_ + START_OF_ARRAY_;
  }


  bool IsCorrectFirstChecker() const {
    
    return first_checker_ == MAGIC_NUM_FOR_FIRST_CHECKER_;
  }


  bool IsCorrectNumOfBlocks() const {
    
    return num_of_blocks_in_memory_ >= GetSizeInBytes(MIN_NUM_OF_ELEMENTS_) &&
           num_of_blocks_in_memory_ >= GetSizeInBytes(size_)                &&
           GetCurrAllStackSize()    == curr_max_size_                       &&
           num_of_blocks_in_memory_ <= MAX_STACK_SIZE_IN_BYTES_;
  }
  
  
  bool IsCorrectSize() const {
    
    return size_ >= 0 && size_ <= GetCurrAllStackSize();
  }


  bool IsCorrectCheckSum() const {
    
    return GetCurrCheckSum() == GetCheckSum();
  }


  bool IsCorrectCanaryEnd() const {
    
    return canary_end_() == CHECKER_CANARY_END_();
  }


  bool IsCorrectCanaryStart() const {
    
    return canary_start_() == CHECKER_CANARY_START_();
  }


  bool ok() const{
    
    return IsCorrectThisPointer()  &&
           IsCorrectArrayPointer() &&
           IsCorrectFirstChecker() &&
           IsCorrectNumOfBlocks()  &&
           IsCorrectSize()         &&
           IsCorrectCheckSum()     &&
           IsCorrectCanaryEnd()    &&
           IsCorrectCanaryStart();
  }


  void Dump() const {
    
    assert(("Incorrect Pointer of this", IsCorrectThisPointer()));
    assert(("Incorrect Pointer of Array", IsCorrectArrayPointer()));
    /*assert(("Somebody have changed memory of stack", IsCorrectFirstChecker()));
    assert(("Problems with memory (incorrect num of blocks in memory)",
            IsCorrectNumOfBlocks()));
    assert(("Problems with memory from left side (wrong start canary)",
            IsCorrectCanaryStart()));
    assert(("Problems with memory from right side (wrong end canary)",
            IsCorrectCanaryEnd()));*/
    std::cout <<"\n=====================================================\n";
    std::cout << "STACK: [" << this << "]";
    StringIsCorrectForDump(IsCorrectThisPointer());
    std::cout <<" {\n\n";
    if (this != nullptr) {
      std::cout << "  first checker is "<<first_checker_;
      StringIsCorrectForDump(IsCorrectFirstChecker());
      std::cout <<"\n";
      std::cout << "  array_[Stack size: " << GetCurrAllStackSize()  
                << "; memory in bytes: "<< num_of_blocks_in_memory_;
      StringIsCorrectForDump(IsCorrectNumOfBlocks());
      std::cout << "], [" << static_cast<void*>(array_) <<"]";
      StringIsCorrectForDump(IsCorrectArrayPointer());
      std::cout << " {\n";
      
      std::cout << "    canary_start_ = 0x" << std::hex << canary_start_() 
                << std::dec << "; index of start is ["
                << GetCanaryStartIndex() << "];";
      StringIsCorrectForDump(IsCorrectCanaryStart());
      std::cout <<"\n\n";

      std::cout << "    element_stack[" << size_;
      StringIsCorrectForDump(IsCorrectSize());
      std::cout <<"]: {\n";
      for(size_t i = 0; i < size_; ++i) {
        std::cout << "      array_[" << i << "] = " 
                  << GetI(i) << "; index of start is ["
                  << GetIndexInStackArray(i) << "];\n";
      }
   
      std::cout << "    }\n    empty_element_stack: {\n";
      for(size_t i = size_; i < GetCurrAllStackSize(); ++i) {
        std::cout << "      array_[" << i << "] = " 
                  << GetI(i) << "; index of start is ["
                  << GetIndexInStackArray(i) << "];\n";
      }
      
      std::cout << "    }\n\n";
      std::cout << "    check_sum = "<< GetCheckSum() 
                << "; index of start is [" 
                << GetCheckSumIndex()
                << "];";
      StringIsCorrectForDump(IsCorrectCheckSum());
      std::cout << "\n\n";

      std::cout << "    canary_end_ = " << canary_end_() 
                << "; index of start is [" 
                << GetCanaryEndIndex()
                << "];";
      StringIsCorrectForDump(IsCorrectCanaryEnd());
      std::cout << "\n\n";

      std::cout << "  }\n}\n";
    }
    std::cout <<"\n=====================================================\n";
  }


  void DebugHelper() const {
    if (debug) {
      if (!ok()) {
        Dump();
        assert(("There is an error in stack, search it in dump", 0));
      }
    }
  }
  //////////////////////////////////////////////////////
 

  void Copy(char* new_array, const ProtectedStack& b, size_t size) {
    
    for (size_t i = 0; i < size; ++i) {

      *reinterpret_cast<T*>(new_array + GetIndexInStackArray(i)) = T();
      *reinterpret_cast<T*>(new_array + GetIndexInStackArray(i)) = b.GetI(i);
    }
  } 


  void Copy(char* new_array, ProtectedStack& b, size_t size) {
    
    for (size_t i = 0; i < size; ++i) {

      *reinterpret_cast<T*>(new_array + GetIndexInStackArray(i)) = 
                            std::move(b.GetI(i));
    }
  } 


  void CopyDebugInfo() {
    
    copy_start_stack_pointer_ = array_ + START_OF_ARRAY_;
    CreateStartCanary();
    CreateEndCanary();
  }


  void ChangeMemoryTo(size_t new_stack_size) {
    
    size_t new_num_of_blocks_in_memory = GetSizeInBytes(new_stack_size);
    char *new_array = new char[new_num_of_blocks_in_memory];
    
    memset(new_array, 0, new_num_of_blocks_in_memory); 
    Copy(new_array, (*this), size_); 
    int curr_check_sum = 0;
    
    if (debug) {
      
      curr_check_sum  = GetCheckSum();
    }
    
    for (int i = 0; i < size_; ++i) {
      DestructI(i);
    }
    delete[] array_;
    curr_max_size_ = new_stack_size; 
    array_ = nullptr;
    num_of_blocks_in_memory_ = new_num_of_blocks_in_memory;
    array_                   = new_array;
    
    if (debug) {
      
      CopyDebugInfo();
      GetCheckSum() = curr_check_sum;
    }
    DebugHelper();
  }


  void reallocate() {
    ChangeMemoryTo(size_ * 2);
  }
  

  void deallocate() {

    ChangeMemoryTo(GetCurrAllStackSize() / 2);
  }
   
  void Initialize() {
     
    size_                    = 0;
    array_                   = nullptr;
    num_of_blocks_in_memory_ = GetSizeInBytes(MIN_NUM_OF_ELEMENTS_);
    array_                   = new char[num_of_blocks_in_memory_];  
    curr_max_size_           = MIN_NUM_OF_ELEMENTS_;
    memset(array_, 0, num_of_blocks_in_memory_);

    if (debug) {

      if (helper_.array_of_stack_starts_.find(CHECKER_THIS()) != 
        helper_.array_of_stack_starts_.end()) {
        assert(("You try to use reallocated memory two times", 0));
      }
      helper_.array_of_stack_starts_.insert(CHECKER_THIS());
      CopyDebugInfo();
      GetCheckSum()         ^= MAGIC_NUM_FOR_CHECKER_SUM_;
      GetCheckSum()         ^= size_;
    }
  }


  void PushHelper(T&& elem) {
    
    GetI(size_) = std::move(elem);
    if (debug) {
      GetCheckSum() ^= hash_function_(GetI(size_)) * (size_ + 1);
      GetCheckSum() ^= size_;
      GetCheckSum() ^= (size_ + 1);
    }
    size_++;
    if (GetLastIndex() == GetEndOFArrayElements()) {
      reallocate();
    }
   
  }


  void DeleteHelper() {
    
    for (int i = 0; i < size_; ++i) {
      DestructI(i);
    }
    delete[] array_;
  }
  

 public:
  
  ProtectedStack() {
    Initialize();
  }

  
  ProtectedStack(const ProtectedStack& another_stack) {
    
    (*this) = another_stack;
    if (debug) {

      if (helper_.array_of_stack_starts_.find(CHECKER_THIS()) != 
        helper_.array_of_stack_starts_.end()) {
        assert(("You try to use reallocated memory two times", 0));
      }
      helper_.array_of_stack_starts_.insert(CHECKER_THIS());
    }
  } 


  ProtectedStack& operator =(const ProtectedStack<T>& another_stack) {
    
    another_stack.DebugHelper();
    
    if (array_ != nullptr) {
      DebugHelper();
    }
    if (array_ == another_stack.array_) {
      return *this;
    }
    
    DeleteHelper();
    size_ = another_stack.size_;
    curr_max_size_ = another_stack.curr_max_size_;
    num_of_blocks_in_memory_ = another_stack.num_of_blocks_in_memory_;
    array_ = new char[num_of_blocks_in_memory_];
    Copy(array_, another_stack, size_); 
     
    if (debug) {
      GetCheckSum()          = another_stack.GetCheckSum();
      CopyDebugInfo();
    }
  }
  

  const T& operator [](size_t i) const {
    
    DebugHelper();
    if (i >= size_ || i < 0) {
      assert(("Current index is out of range of stack", 0));
    }
    return GetI(i);
  }


  const T& Back() const {
    
    DebugHelper();
    if (size_ == 0) {
      assert(("Stack is empty, you try to get last index from it", 0)); 
    }
    return GetI(size_ - 1); 
  }
  

  void PushBack(const T& elem) {
    
    DebugHelper();
    T copy = elem;
    PushHelper(std::move(copy)); 
    DebugHelper();
  }
 

  void PushBack(T&& elem) {
    
    DebugHelper();
    PushHelper(std::forward<T>(elem));
    DebugHelper();
  }


  void PopBack() {
  
    DebugHelper();
    
    if (debug) {
      GetCheckSum() ^= hash_function_(Back()) * (size_);
      GetCheckSum() ^= size_;
      GetCheckSum() ^= (size_ - 1);
    }
    
    if (size_ == 0) {
      assert(0);
    }
    
    DestructI(size_);
    --size_;

    int curr_stack_size = GetCurrAllStackSize();
    if (size_ < curr_stack_size / 3 && 
        curr_stack_size / 2 >= MIN_NUM_OF_ELEMENTS_) {
      deallocate();
    }
    DebugHelper();
  }
  

  size_t Size() {
    
    DebugHelper();    
    return size_;
  }
    

  bool Empty() const {
    
    DebugHelper();
    return !size_;
  }


  void PrintForDebug() const {
    if (debug) {
      Dump();
    }
  }


  void Clear() {
    
    DebugHelper();
    DeleteHelper();
    Initialize();
  }


  ~ProtectedStack() {

    DebugHelper();
    DeleteHelper();
    helper_.array_of_stack_starts_.erase(CHECKER_THIS());
  }
}; 

