/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

namespace Alg {

static const ADGameDescription gameDescriptions[] = {
	{
		// Mad Dog McCree (v1.03a)
		"maddog",
		"",
		AD_ENTRY1s("MADDOG.LIB", "df27e760531dba600cb3ebc23a2d98d1", 114633310),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Mad Dog II: The Lost Gold (v1.00) (Single Speed CD-ROM Version)
		"maddog2s",
		"",
		AD_ENTRY1s("MADDOG2.LIB", "7b54bca3932b28d8776eaed16a9f43b5", 185708043),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Mad Dog II: The Lost Gold (v1.00) (Double Speed CD-ROM Version)
		"maddog2d",
		"",
		AD_ENTRY1s("MADDOG2D.LIB", "1660b1728573481483c50206ad92a0ca", 291119013),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Who Shot Johnny Rock? (v1.00) (Single Speed CD-ROM Version)
		"johnrocs",
		"",
		AD_ENTRY1s("JOHNROC.LIB", "3cbf7843ef2fdf23716301dceaa2eb10", 141833752),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Who Shot Johnny Rock? (v1.00) (Double Speed CD-ROM Version)
		"johnrocd",
		"",
		AD_ENTRY1s("JOHNROCD.LIB", "93c38b5fc7d1ae6e9dccf4f7a1c313a8", 326535618),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Alg
