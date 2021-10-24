#ifndef TELOMERE_HPP_INCLUDED
#define TELOMERE_HPP_INCLUDED

#include "base.hpp"

#include <array>

namespace dna {

static constexpr auto Telomere = std::array {
    dna::T,
    dna::T,
    dna::A,
    dna::G,
    dna::G,
    dna::G
};

constexpr auto TelomereSize = std::size(Telomere);

}

#endif
