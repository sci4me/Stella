namespace otr {
	template<typename A, typename B>
	constexpr bool type_eq = false;

	template<typename A>
	constexpr bool type_eq<A, A> = true;
}