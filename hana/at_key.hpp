/*!
@file
Defines `boost::hana::at_key`.

@copyright Louis Dionne 2013-2016
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_HANA_AT_KEY_HPP
#define BOOST_HANA_AT_KEY_HPP

#include <hana/fwd/at_key.hpp>

#include <hana/accessors.hpp>
#include <hana/at.hpp>
#include <hana/concept/searchable.hpp>
#include <hana/concept/struct.hpp>
#include <hana/config.hpp>
#include <hana/core/dispatch.hpp>
#include <hana/detail/decay.hpp>
#include <hana/equal.hpp>
#include <hana/find.hpp>
#include <hana/find_if.hpp>
#include <hana/first.hpp>
#include <hana/functional/on.hpp>
#include <hana/length.hpp>
#include <hana/optional.hpp>
#include <hana/second.hpp>

#include <cstddef>
#include <utility>


BOOST_HANA_NAMESPACE_BEGIN
    //! @cond
    template <typename Xs, typename Key>
    constexpr decltype(auto) at_key_t::operator()(Xs&& xs, Key const& key) const {
        using S = typename hana::tag_of<Xs>::type;
        using AtKey = BOOST_HANA_DISPATCH_IF(at_key_impl<S>,
            hana::Searchable<S>::value
        );

    #ifndef BOOST_HANA_CONFIG_DISABLE_CONCEPT_CHECKS
        static_assert(hana::Searchable<S>::value,
        "hana::at_key(xs, key) requires 'xs' to be Searchable");
    #endif

        return AtKey::apply(static_cast<Xs&&>(xs), key);
    }
    //! @endcond

    template <typename S, bool condition>
    struct at_key_impl<S, when<condition>> : default_ {
        template <typename Xs, typename Key>
        static constexpr auto apply(Xs&& xs, Key const& key) {
            return hana::find(static_cast<Xs&&>(xs), key).value();
        }
    };

    namespace at_key_detail {
        template <typename T>
        struct equal_to {
            T const& t;
            template <typename U>
            constexpr auto operator()(U const& u) const {
                return hana::equal(t, u);
            }
        };

        //! @todo This causes an awful duplication of code with `find_if`.
        template <typename Xs, typename Pred, std::size_t i, std::size_t N, bool Done>
        struct advance_until;

        template <typename Xs, typename Pred, std::size_t i, std::size_t N>
        struct advance_until<Xs, Pred, i, N, false>
            : advance_until<Xs, Pred, i + 1, N, static_cast<bool>(detail::decay<decltype(
                std::declval<Pred>()(hana::at_c<i>(std::declval<Xs>()))
            )>::type::value)>
        { };

        template <typename Xs, typename Pred, std::size_t N>
        struct advance_until<Xs, Pred, N, N, false> {
            template <typename Ys>
            static constexpr auto apply(Ys&&) = delete;
        };

        template <typename Xs, typename Pred, std::size_t i, std::size_t N>
        struct advance_until<Xs, Pred, i, N, true> {
            template <typename Ys>
            static constexpr decltype(auto) apply(Ys&& ys) {
                return hana::at_c<i - 1>(static_cast<Ys&&>(ys));
            }
        };
    }

    template <typename S>
    struct at_key_impl<S, when<hana::Sequence<S>::value>> {
        template <typename Xs, typename Key>
        static constexpr decltype(auto) apply(Xs&& xs, Key const&) {
            constexpr std::size_t N = decltype(hana::length(xs))::value;
            using Pred = at_key_detail::equal_to<Key>;
            return at_key_detail::advance_until<Xs&&, Pred, 0, N, false>::apply(
                static_cast<Xs&&>(xs)
            );
        }
    };

    template <typename S>
    struct at_key_impl<S, when<hana::Struct<S>::value>> {
        template <typename X, typename Key>
        static constexpr decltype(auto) apply(X&& x, Key const& key) {
            auto accessor = hana::second(*hana::find_if(hana::accessors<S>(),
                hana::equal.to(key) ^hana::on^ hana::first
            ));
            return accessor(static_cast<X&&>(x));
        }
    };
BOOST_HANA_NAMESPACE_END

#endif // !BOOST_HANA_AT_KEY_HPP
