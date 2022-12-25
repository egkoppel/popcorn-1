- Inheritance is for when a thing is another thing. Composition is when a thing has another thing
    - A multiboot tag contains a header - therefore it contains a `header` field
    - A monotonic allocator is an allocator - therefore it inherits from `IAllocator`
- `struct`s imply a value type. `class`es imply a resource owning object type.
    - `struct`s should
        - be passed by value
        - have all public members
        - have a camel_case name ending in `_t`
        - have all default constructors and assignment operators
        - not own resources
    - `class`es should
        - be passed by `const` reference unless necessary
        - usually have all private members
        - have a PascalCase name
        - own resources
        - constructors:
            - default constructors should be of the form `T()`, and create an 'empty' but still valid object
            - copy constructor should be of the form `T(const T&)`, and perform a deep copy of the object, including
              allocating duplicate resources
            - copy constructors should be deleted when resources are unique
            - move constructor should be of the form `T(T&&) noexcept`, and perform a move of any resources. destruction
              of the moved-from object should not cause invalidation of the moved-into object. if a default constructor
              exists, consider implementing the move constructor as
              ```c++
              T(T&& other) : T() { std::swap(*this, other); }
              ```
            - if the object is a container type, consider implementing a constructor of the
              form `T(std::initialiser_list<U>)`
            - if a class contains a constructor of the form `T(U a, V b, ...) : a(a), b(b), ... {}`, consider whether it
              should be a `struct` instead
            - if a `class` requires special
        - assignment operators:
            - if a copy/move constructor exists, an equivalent assignment operator should exist
            - copy assignment should be of the form `T& operator=(const T&)` and should ensure any resources are not
              leaked
            - move assignment should be of the form `T& operator=(T&&)` - consider implementing as
              ```c++
              T& operator=(T&& rhs) { std::swap(this->a, rhs.a); ... }
              ```
        - destructors should clean up any resources from the constructor
        - if `private`/`protected` inheritance is needed, think better
        - `class` declaration order should follow one of the following specifications
          ```c++
          template<class T> MyObject : public IObjectifiable,
                                       public BaseObject,
                                       public mixin::post_increment<MyObject<T>> {
              friend void foo();
              friend class Bar;
          public:
              class iterator { ... };
          
              MyObject();
              MyObject(size_t object_count);
              MyObject(std::initializer_list<T> objects);
              MyObject(const MyObject&);
              MyObject(MyObject&&) noexcept;
              ~MyObject();
          
              T& operator=(const T&);
              T& operator=(T&&) noexcept;
          
              bool operator!=(const MyObject&) const;
              std::ordering operator<=>(const MyObject&) const;
          
              MyObject& operator+=(const MyObject&);
              MyObject& operator-=(const MyObject&);
          
              MyObject& operator++();
              MyObject& operator--();
          
              iterator begin();
              iterator end();
          
              // Any non-standard member functions
          
          protected:
          private:
              size_t capacity;
          
		      // Any private member functions
          };
          
          class MyComplexObject {
		  private:
              MyComplexObject(...);
          public:
              static MyComplexObject create_complex_object(...);
          
              // Rest as above
		  };
          ```
        - mixins should be used for declaring operators where possible
        - if using a `friend` class, consider whether it could be replaced with a member class
- use references for borrowing, `std::unique_ptr` for ownership
    - `std::shared_ptr` should have very careful thought about it if it seems necessary
    - borrowing of a heap object should be to `T&` not `unique_ptr<T>&` - this guarantees that the reference does not
      become invalidated until the end of the object's lifetime
    - `class`es and `struct`s should borrow through `T*` or `std::optional<T*>` to allow for correct operation of
      assignment operators. `T*` implies non-nullable - use `std::optional<T*>` where nullability is needed.
      constructors to the borrowing object should take by `T&` and convert to `T*` internally
- resources should be owned by a handle to the resource
    - destruction of the handle should release the resource - if a manual release function is required to be called,
      think better
    - copying the handle should acquire a new resource and copy any underlying data into it
- construct classes with `T obj{}` or `T obj{...}` for constructors, and `T obj = { ... }` for initialiser list
- construct structs with `t_t data{ .a = ..., .b = ...}`
- return by constructing as just `return { ... }` where possible (with `.field =` syntax for returning structs)