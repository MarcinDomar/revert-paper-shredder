#pragma once
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
