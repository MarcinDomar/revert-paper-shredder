#pragma once
/*
Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2020 Marcin Domarski.
Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
class PresenceRegister
{
	unsigned int registry = 0;
public:
	PresenceRegister() = default;
	
	template <int size, typename ColIndexType>
	PresenceRegister(const std::array<ColIndexType, size> & elements) {
		add(elements);
	}

	template <int size, typename ColIndexType>
	void add(const std::array<ColIndexType, size> &elements) {
		for (auto e : elements) registry |= 1u << e;
	}
	
	void add(const PresenceRegister & reg) {
		registry |= reg.registry;
	}
	
	void subtract(const PresenceRegister & reg) {
		registry &= ~reg.registry;
	}

	bool isThereCommonPart(const PresenceRegister & reg) const {
		return (registry& reg.registry) > 0;
	}

	void reset() { registry = 0u; }

	template <typename ColIndexType>
	std::vector<ColIndexType> getMissingColumns(int columnsSize) {
		std::vector<ColIndexType> ret;
		for (int i = 0; i < columnsSize; i++) {
			if ((registry& (1u << i)) == 0u)
				ret.push_back(i);
		}
		return ret;
	}
};
