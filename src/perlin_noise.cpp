// NOTE: I have mutilated the original version of this code because it was shit.

//----------------------------------------------------------------------------------------
//
//	siv::PerlinNoise
//	Perlin noise library for modern C++
//
//	Copyright (C) 2013-2020 Ryo Suzuki <reputeless@gmail.com>
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files(the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions :
//	
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//	
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.
//
//----------------------------------------------------------------------------------------

struct PerlinNoise {
	static constexpr f32 fade(f32 t) noexcept {
		return t * t * t * (t * (t * 6 - 15) + 10);
	}

	static constexpr f32 grad(u8 hash, f32 x, f32 y, f32 z) noexcept {
		const u8 h = hash & 15;
		const f32 u = h < 8 ? x : y;
		const f32 v = h < 4 ? y : h == 12 || h == 14 ? x : z;
		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}

	static constexpr f32 weight(s32 octaves) noexcept {
		f32 amp = 1;
		f32 value = 0;

		for (s32 i = 0; i < octaves; ++i) {
			value += amp;
			amp /= 2;
		}

		return value;
	}

	u8 p[512];

	void reseed(u32 seed) {
		for(u16 i = 0; i < 256; i++) p[i] = (u8)i;
		
		rnd_pcg_t prng;
		rnd_pcg_seed(&prng, seed);
		for(u16 i = 255; i > 0; i--) {
			s32 j = rnd_pcg_range(&prng, 0, i);
			u8 t = p[i];
			p[i] = p[j];
			p[j] = t;
		}

		for(u16 i = 0; i < 256; i++) p[256 + i] = p[i];
	}

	f32 noise1D(f32 x) const noexcept {
		return noise3D(x, 0, 0);
	}

	f32 noise2D(f32 x, f32 y) const noexcept {
		return noise3D(x, y, 0);
	}

	f32 noise3D(f32 x, f32 y, f32 z) const noexcept {
		const s32 X = ((s32)(floorf(x))) & 255;
		const s32 Y = ((s32)(floorf(y))) & 255;
		const s32 Z = ((s32)(floorf(z))) & 255;

		x -= floorf(x);
		y -= floorf(y);
		z -= floorf(z);

		const f32 u = fade(x);
		const f32 v = fade(y);
		const f32 w = fade(z);

		const s32 A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z;
		const s32 B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;

		return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
			grad(p[BA], x - 1, y, z)),
			lerp(u, grad(p[AB], x, y - 1, z),
			grad(p[BB], x - 1, y - 1, z))),
			lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
			grad(p[BA + 1], x - 1, y, z - 1)),
			lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
			grad(p[BB + 1], x - 1, y - 1, z - 1))));
	}

	f32 noise1D_0_1(f32 x) const noexcept {
		return noise1D(x) * f32(0.5) + f32(0.5);
	}

	f32 noise2D_0_1(f32 x, f32 y) const noexcept {
		return noise2D(x, y) * f32(0.5) + f32(0.5);
	}

	f32 noise3D_0_1(f32 x, f32 y, f32 z) const noexcept {
		return noise3D(x, y, z) * f32(0.5) + f32(0.5);
	}

	f32 accumulatedOctaveNoise1D(f32 x, s32 octaves) const noexcept {
		f32 result = 0;
		f32 amp = 1;

		for (s32 i = 0; i < octaves; ++i) {
			result += noise1D(x) * amp;
			x *= 2;
			amp /= 2;
		}

		return result;
	}

	f32 accumulatedOctaveNoise2D(f32 x, f32 y, s32 octaves) const noexcept {
		f32 result = 0;
		f32 amp = 1;

		for (s32 i = 0; i < octaves; ++i) {
			result += noise2D(x, y) * amp;
			x *= 2;
			y *= 2;
			amp /= 2;
		}

		return result;
	}

	f32 accumulatedOctaveNoise3D(f32 x, f32 y, f32 z, s32 octaves) const noexcept {
		f32 result = 0;
		f32 amp = 1;

		for (s32 i = 0; i < octaves; ++i) {
			result += noise3D(x, y, z) * amp;
			x *= 2;
			y *= 2;
			z *= 2;
			amp /= 2;
		}

		return result;
	}

	f32 normalizedOctaveNoise1D(f32 x, s32 octaves) const noexcept {
		return accumulatedOctaveNoise1D(x, octaves) / weight(octaves);
	}

	f32 normalizedOctaveNoise2D(f32 x, f32 y, s32 octaves) const noexcept {
		return accumulatedOctaveNoise2D(x, y, octaves) / weight(octaves);
	}

	f32 normalizedOctaveNoise3D(f32 x, f32 y, f32 z, s32 octaves) const noexcept {
		return accumulatedOctaveNoise3D(x, y, z, octaves) / weight(octaves);
	}

	f32 accumulatedOctaveNoise1D_0_1(f32 x, s32 octaves) const noexcept {
		return clamp<f32>(accumulatedOctaveNoise1D(x, octaves) * f32(0.5) + f32(0.5), 0, 1);
	}

	f32 accumulatedOctaveNoise2D_0_1(f32 x, f32 y, s32 octaves) const noexcept {
		return clamp<f32>(accumulatedOctaveNoise2D(x, y, octaves) * f32(0.5) + f32(0.5), 0, 1);
	}

	f32 accumulatedOctaveNoise3D_0_1(f32 x, f32 y, f32 z, s32 octaves) const noexcept {
		return clamp<f32>(accumulatedOctaveNoise3D(x, y, z, octaves) * f32(0.5) + f32(0.5), 0, 1);
	}

	f32 normalizedOctaveNoise1D_0_1(f32 x, s32 octaves) const noexcept {
		return normalizedOctaveNoise1D(x, octaves) * f32(0.5) + f32(0.5);
	}

	f32 normalizedOctaveNoise2D_0_1(f32 x, f32 y, s32 octaves) const noexcept {
		return normalizedOctaveNoise2D(x, y, octaves) * f32(0.5) + f32(0.5);
	}

	f32 normalizedOctaveNoise3D_0_1(f32 x, f32 y, f32 z, s32 octaves) const noexcept {
		return normalizedOctaveNoise3D(x, y, z, octaves) * f32(0.5) + f32(0.5);
	}

	void serialize(Static_Array<u8, 256>& s) const noexcept {
		for (u32 i = 0; i < 256; ++i) {
			s[i] = p[i];
		}
	}

	void deserialize(Static_Array<u8, 256> const& s) noexcept {
		for (u32 i = 0; i < 256; ++i) {
			p[256 + i] = p[i] = s[i];
		}
	}
};