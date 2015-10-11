#include <tuple>
#include <type_traits>


namespace graph {

    struct readable_property_map_tag { };

    struct writable_property_map_tag { };

    struct read_write_property_map_tag :
        public readable_property_map_tag,
        public writable_property_map_tag { };

    struct lvalue_property_map_tag :
        public read_write_property_map_tag { };

    template <typename Graph, typename Tag, typename EnableIf = void>
    struct property_map;

    template <typename Graph, typename Tag, typename EnableIf>
    using property_map_t = typename property_map<Graph, Tag, EnableIf>::type;

    template<typename PMap> 
    struct property_traits {
        using value_type = typename PMap::value_type;
        using reference  = typename PMap::reference;
        using key_type   = typename PMap::key_type;
        using category   = typename PMap::category;
    };

    template <typename Tag, typename Value>
    struct Property:public std::tuple<Value> {
        using Base = std::tuple<Value>;
        using tag_type = Tag;
        Property(const Value& v) :Base(v) {};
        Property() :Base() {};
    };
    
    
    template <typename... Ps>
    struct Properties:public std::tuple<Ps...> {
        using Base = std::tuple<Ps...>;
    };

    namespace detail {
        template <typename T, typename U>
        using IsSame_t = typename std::is_same<T, U>::type;

        template <typename Tag, typename Props, size_t I = 0>
        struct FindPropertyByTag;

        template <typename Tag, typename Props, size_t I = 0>
        using FindPropertyByTag_t = typename FindPropertyByTag<Tag, Props, I>::type;

        template < typename Tag, size_t I, typename P, typename... Ps >
        struct FindPropertyByTag < Tag, Properties<P, Ps...>, I > {
            using type = std::conditional_t <
                IsSame_t<Tag, typename P::tag_type>::value, 
                std::integral_constant<size_t, I>, 
                FindPropertyByTag_t<Tag, Properties<Ps...>, I + 1 >
            >;
        };

        template < typename Tag, size_t I >
        struct FindPropertyByTag < Tag, Properties < >, I > {
            using type = void;
        };
    };


    // functions to access properties by tag (correspond to tuple's getters)

    template <typename Tag, typename... Ps> 
    decltype(auto) get(Properties<Ps...>& props) {
        using Props = Properties<Ps...>;
        using Index = detail::FindPropertyByTag_t<Tag, Props>;
        using P = std::tuple_element<Index::value, Props::Base>::type;
        return std::get<0>(static_cast<P::Base&>(std::get<Index::value>(static_cast<Props::Base&>(props))) );
    };

    template <typename Tag, typename... Ps>
    decltype(auto) get(Properties<Ps...>&& props) {
        using Props = Properties<Ps...>;
        using Index = detail::FindPropertyByTag_t<Tag, Props>;
        using P = std::tuple_element<Index::value, Props::Base>::type;
        return std::get<0>(static_cast<P::Base&&>(std::get<Index::value>(static_cast<Props::Base&&>(props))));
    };

    template <typename Tag, typename... Ps>
    decltype(auto) get(const Properties<Ps...>& props) {
        using Props = Properties<Ps...>;
        using Index = detail::FindPropertyByTag_t<Tag, Props>;
        using P = std::tuple_element<Index::value, Props::Base>::type;
        return std::get<0>(static_cast<const P::Base&>(std::get<Index::value>(static_cast<const Props::Base&>(props))));
    };

    //bundled properties
    struct vertex_bundle_t {};
    struct edge_bundle_t {};


    namespace detail {
        template <typename Tag, typename BaseBundledPM>
        class BundledSubPropertyMap: public BaseBundledPM {
            using Props = typename property_traits<BaseBundledPM>::value_type;
            using Index = detail::FindPropertyByTag_t<Tag, Props>;
        public:
            using value_type = typename std::tuple_element<0,typename std::tuple_element<Index::value, typename Props::Base>::type::Base>::type;
            using reference = value_type&;
            using key_type = typename BaseBundledPM::key_type;
            using category = typename BaseBundledPM::category;
            BundledSubPropertyMap(BaseBundledPM& base) :BaseBundledPM(base) {};
            BundledSubPropertyMap():BaseBundledPM() {};
        };

        template < typename Graph >
        class IsGraph {
            template < typename C, typename = C::vertex_descriptor>
                static std::true_type check(const C&) {};
            static std::false_type check(...) {};
        public:
            using type = decltype(check(std::declval<Graph>()));
            static const bool value = type::value;
        };

        template <typename Graph, typename Tag, typename BundleTag, typename EnableIf = void>
        struct IsTagNotInBundleTag : public std::true_type {};
        
        template <typename Graph, typename Tag>
        struct IsTagNotInBundleTag<Graph,Tag,vertex_bundle_t, std::enable_if_t<IsGraph<Graph>::value>>:
            public std::is_same<
                void,
                detail::FindPropertyByTag_t<
                    Tag, 
                    typename property_traits<typename property_map<Graph,vertex_bundle_t>::type>::value_type>
            >{};
        template <typename Graph, typename Tag>
        struct IsTagNotInBundleTag<Graph,Tag,edge_bundle_t, std::enable_if_t<IsGraph<Graph>::value>>:
            public std::is_same<
                void,
                detail::FindPropertyByTag_t<
                    Tag, 
                    typename property_traits<typename property_map<Graph,edge_bundle_t>::type>::value_type>
            >{};

    };

    template <class Graph, class Tag>
    struct property_map<Graph, Tag, std::enable_if_t<!detail::IsTagNotInBundleTag<Graph, Tag, vertex_bundle_t>::value>> {
        using type = detail::BundledSubPropertyMap<Tag, typename property_map<Graph, vertex_bundle_t>::type>;
    };

    template <class Graph, class Tag>
    struct property_map<Graph, Tag, std::enable_if_t<!detail::IsTagNotInBundleTag<Graph, Tag, edge_bundle_t>::value>> {
        using type = detail::BundledSubPropertyMap<Tag, typename property_map<Graph, edge_bundle_t>::type>;
    };

    template <typename Graph, typename Tag> 
    typename property_map<Graph, Tag>::type get(Tag, Graph& graph, 
        std::enable_if_t<!detail::IsTagNotInBundleTag<Graph, Tag, vertex_bundle_t>::value>* = nullptr) {
        using VertexBundledPM = typename property_map<Graph, vertex_bundle_t>::type;
        return detail::BundledSubPropertyMap<Tag, VertexBundledPM>(graph::get(vertex_bundle_t(), graph));
    };

    template <typename Graph, typename Tag>
    typename property_map<Graph, Tag>::type get(Tag, Graph& graph,
        std::enable_if_t<!detail::IsTagNotInBundleTag<Graph, Tag, edge_bundle_t>::value>* = nullptr) {
        using VertexBundledPM = typename property_map<Graph, edge_bundle_t>::type;
        return detail::BundledSubPropertyMap<Tag, VertexBundledPM>(graph::get(edge_bundle_t(), graph));
    };

 

    template <typename Tag, typename BaseBundledPM>
    typename detail::BundledSubPropertyMap<Tag, BaseBundledPM>::reference get(
        const detail::BundledSubPropertyMap<Tag, BaseBundledPM>& pMap,
        const typename detail::BundledSubPropertyMap<Tag, BaseBundledPM>::key_type& key) {        
        return graph::get<Tag>(graph::get(static_cast<const BaseBundledPM&>(pMap), key));
    }


    template <typename Tag, typename BaseBundledPM>
    void  put(
        detail::BundledSubPropertyMap<Tag, BaseBundledPM>& pMap,
        const typename detail::BundledSubPropertyMap<Tag, BaseBundledPM>::key_type& key,
        const typename detail::BundledSubPropertyMap<Tag, BaseBundledPM>::value_type& value) {
        get<Tag>(get(static_cast<BaseBundledPM&>(pMap), key)) = value;
    }
}