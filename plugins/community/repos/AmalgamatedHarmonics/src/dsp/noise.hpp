#pragma once

#include <random>

namespace rack_plugin_AmalgamatedHarmonics {
	namespace bogaudio_dsp {

		struct Generator {
			float _current = 0.0;

			Generator() {}
			virtual ~Generator() {}

			float current() {
				return _current;
			}

			float next() {
				return _current = _next();
			}

			virtual float _next() = 0;
			
		};

		class Seeds {
		private:
			
			std::mt19937 _generator;
			
			Seeds() {
				std::random_device rd;
				_generator.seed(rd());
			}

			unsigned int _next() {
				return _generator();
			}

		public:
			Seeds(const Seeds&) = delete;
			void operator=(const Seeds&) = delete;
			
			static Seeds& getInstance() {
				static Seeds instance;
				return instance;
			}
			
			static unsigned int next() {
				return getInstance()._next();
			};
		};

		struct NoiseGenerator : Generator {
			std::minstd_rand _generator; // one of the faster options.
			NoiseGenerator() : _generator(Seeds::next()) {}
		};

		struct WhiteNoiseGenerator : NoiseGenerator {
			std::uniform_real_distribution<float> _uniform;

			WhiteNoiseGenerator() : _uniform(-1.0, 1.0) {}

			virtual float _next() override {
				return _uniform(_generator);
			}
		};

		template<typename G>
		struct BasePinkNoiseGenerator : NoiseGenerator {
			static const int _n = 6;
			G _g;
			G _gs[_n];
			uint32_t _count = _g.next();

			virtual float _next() override {
				// See: http://www.firstpr.com.au/dsp/pink-noise/
				float sum = _g.next();
				for (int i = 0, bit = 1; i < _n; ++i, bit <<= 1) {
					if (_count & bit) {
						sum += _gs[i].next();
					}
					else {
						sum += _gs[i].current();
					}
				}
				++_count;
				return sum / (float)(_n + 1);
			}
		};

		struct PinkNoiseGenerator : BasePinkNoiseGenerator<WhiteNoiseGenerator> {};

		struct RedNoiseGenerator : BasePinkNoiseGenerator<PinkNoiseGenerator> {};

		struct GaussianNoiseGenerator : NoiseGenerator {
			std::normal_distribution<float> _normal;

			GaussianNoiseGenerator() : _normal(0, 1.0) {}

			virtual float _next() override {
				return _normal(_generator);
			}
		};

	} // namespace bogaudio_dsp
} // namespace rack_plugin_AmalgamatedHarmonics
