// NOTE: Only needed if we are using placement new.
inline void* operator new(size_t, void* ptr) { return ptr; }
inline void  operator delete(void*, void*) {}
	
namespace otr {
	template<typename A, typename B>
	constexpr bool type_eq = false;

	template<typename A>
	constexpr bool type_eq<A, A> = true;
}