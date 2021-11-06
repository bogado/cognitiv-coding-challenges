#include "./fake_person_fixture.hpp"

namespace fixtures {

const fake_person fake_person_fixture_std{make_fake_person()};
const fake_person fake_person_fixture_mut{make_fake_person<mutations>()};

}

