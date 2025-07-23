#ifndef ALH_RAND_HPP
#define ALH_RAND_HPP

#include <cstdint>
#include <cstring>

namespace alh {

    class rand_f32 {
        // used for seeding
        static inline uint32_t splitmix32(uint32_t &s) {
            uint32_t z = (s += 0x9e3779b9);
            z ^= z >> 16; z *= 0x21f0aaad;
            z ^= z >> 15; z *= 0x735a2d97;
            z ^= z >> 15;
            return z;
        }
        
        // rng implementation from xoshiro-128+
        static inline uint32_t rotl(const uint32_t x, int k) {
            return (x << k) | (x >> (32 - k));
        }

        uint32_t state[4];

        uint32_t next(void) {
            const uint32_t result = state[0] + state[3];

            const uint32_t t = state[1] << 9;

            state[2] ^= state[0];
            state[3] ^= state[1];
            state[1] ^= state[2];
            state[0] ^= state[3];

            state[2] ^= t;

            state[3] = rotl(state[3], 11);

            return result;
        }

    public:
        void seed(uint32_t s) {
            // use 4 steps of splitmix32 to seed xoshiro-128+
            state[0] = splitmix32(s);
            state[1] = splitmix32(s);
            state[2] = splitmix32(s);
            state[3] = splitmix32(s);
        }

        float get() {
            // get next value, extract upper 23-bits, and convert to float in the range [0, 1)
            //return (float)(0x3f800000 | next() >> 9) - 1.f;
            uint32_t in = (0x3f800000 | next() >> 9);
            float out;
            memcpy(&out, &in, 4);
            return out - 1.f;
        }

        float get_uniform(float min, float max) {
            // get float in the range [min, max) from uniform distribution
            return min + (max - min) * get();
        }

        float get_tri(float min, float max) {
            // get float in the range [min, max) from triangular distribution
            return min + (max - min) * (get() - get() + 1.f) / 2.f;
        } 

        float get_normalish(float min, float max) {
            // get float in the range [min, max) from normal-ish (gaussian-esque if u will) distribution
            return min + (max - min) * (get() - get() + get() - get() + 2.f) / 4.f;
        }
    };

} // namespace alh

#endif