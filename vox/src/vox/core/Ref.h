#pragma once

// thank you cherno
namespace vox {
    class RefCounted {
    public:
        virtual ~RefCounted() = default;

        void IncreaseRefCount() const { m_RefCount++; }
        void DecreaseRefCount() const { m_RefCount--; }

        uint32_t GetRefCount() const { return m_RefCount.load(); }

    private:
        mutable std::atomic<uint32_t> m_RefCount = 0;
    };

    template <typename T>
    class Ref {
    public:
        Ref() { m_Instance = nullptr; }
        Ref(std::nullptr_t) { m_Instance = nullptr; }

        Ref(T* instance) {
            static_assert(std::is_base_of_v<RefCounted, T>,
                          "Template argument is not derived from RefCounted!");

            m_Instance = instance;
            IncreaseRefCount();
        }

        Ref(const Ref<T>& other) {
            m_Instance = other.m_Instance;
            IncreaseRefCount();
        }

        Ref(Ref<T>&& other) {
            m_Instance = other.m_Instance;
            other.m_Instance = nullptr;
        }

        ~Ref() { DecreaseRefCount(); }

        Ref<T>& operator=(std::nullptr_t) {
            DecreaseRefCount();
            m_Instance = nullptr;

            return *this;
        }

        Ref<T>& operator=(const Ref<T>& other) {
            if (this == &other) {
                return *this;
            }

            DecreaseRefCount();
            m_Instance = other.m_Instance;
            IncreaseRefCount();

            return *this;
        }

        Ref<T>& operator=(Ref<T>&& other) {
            DecreaseRefCount();

            m_Instance = other.m_Instance;
            other.m_Instance = nullptr;

            return *this;
        }

        operator bool() { return m_Instance != nullptr; };
        operator bool() const { return m_Instance != nullptr; }

        T* operator->() { return m_Instance; }
        const T* operator->() const { return m_Instance; }

        T& operator*() { return *m_Instance; }
        const T& operator*() const { return *m_Instance; }

        T* Get() { return m_Instance; }
        const T* Get() const { return m_Instance; }

        void Reset(T* instance = nullptr) {
            DecreaseRefCount();
            m_Instance = instance;
            IncreaseRefCount();
        }

        template <typename U>
        Ref<U>&& As() {
            return Ref<U>(dynamic_cast<U>(m_Instance));
        }

        template <typename... Args>
        static Ref<T> Create(Args&&... args) {
            return Ref<T>(new T(std::forward<Args>(args)...));
        }

        bool operator==(const Ref<T>& other) const { return m_Instance == other.m_Instance; }
        bool operator!=(const Ref<T>& other) const { return m_Instance != other.m_Instance; }

    private:
        void IncreaseRefCount() const {
            if (m_Instance != nullptr) {
                m_Instance->IncreaseRefCount();
            }
        }

        void DecreaseRefCount() const {
            if (m_Instance != nullptr) {
                m_Instance->DecreaseRefCount();
                if (m_Instance->GetRefCount() == 0) {
                    delete m_Instance;
                    m_Instance = nullptr;
                }
            }
        }

        mutable T* m_Instance;
    };
} // namespace vox