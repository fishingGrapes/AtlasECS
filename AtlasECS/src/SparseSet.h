#pragma once

#include <vector>

template<typename T>
class SparseSet
{
	using iterator = typename std::vector<T>::iterator;
	using const_iterator = typename std::vector<T>::const_iterator;

public:

	inline size_t size() const { return m_Size; }
	inline size_t capacity() const { return m_Capacity; }

	bool empty() const { return m_Size == 0; }
	void clear() { m_Size = 0; }

	void reserve(size_t cap)
	{
		if (cap > m_Capacity)
		{
			m_DenseArray.resize(cap, 0);
			m_SparseArray.resize(cap, 0);
			m_Capacity = cap;
		}
	}

	bool has(const T& val) const
	{
		return val < m_Capacity && m_SparseArray[val] < m_Size && m_DenseArray[m_SparseArray[val]] == val;
	}

	void insert(const T& val)
	{
		if (!has(val))
		{
			if (val >= m_Capacity)
				reserve(val + 1);

			m_DenseArray[m_Size] = val;
			m_SparseArray[val] = static_cast<T>(m_Size);
			++m_Size;
		}
	}

	void erase(const T& val)
	{
		if (has(val))
		{
			m_DenseArray[m_SparseArray[val]] = m_DenseArray[m_Size - 1];
			m_SparseArray[m_DenseArray[m_Size - 1]] = m_SparseArray[val];
			--m_Size;
		}
	}

	iterator begin() { return m_DenseArray.begin(); }
	iterator end() { return m_DenseArray.begin() + m_Size; }

	const_iterator begin() const { return m_DenseArray.begin(); }
	const_iterator end() const { return m_DenseArray.end() + m_Size; }

private:
	std::vector<T> m_DenseArray;
	std::vector<T> m_SparseArray;

	size_t m_Size = 0;			// current number of elements
	size_t m_Capacity = 0;		// available memory

};