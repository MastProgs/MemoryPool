#pragma once
#include <vector>
#include <map>
#include <unordered_map>

class MyFreeList
{
	using BYTE = unsigned char;
	using Spot = std::pair<size_t, size_t>;

public:
	MyFreeList() = delete;

	MyFreeList(const size_t & blockSize)
		: m_blockSize{ blockSize }
	{
		m_memList.emplace_back(reinterpret_cast<BYTE *>(malloc(m_blockSize)));
		m_memList.emplace_back(reinterpret_cast<BYTE *>(malloc(m_blockSize)));
		m_memSize = 0b10;
		m_freeIndex.insert(std::make_pair(0, 0));
		m_freeIndex.insert(std::make_pair(1, 0));
	}

	~MyFreeList()
	{
		for (auto & d : m_memList)
		{
			free(d);
		}
	}

	void Free(BYTE * p)
	{
		auto iter = m_usingPage.find(reinterpret_cast<BYTE *>(p));
		auto vectIndex = iter->second.first;
		if (iter == m_usingPage.end()) { while (true); };	// error
		m_freeIndex.insert(std::make_pair(vectIndex, iter->second.second));
		m_usingPage.erase(iter);

		if (m_usingPage.size() < (m_memSize / 4))
		{
			// 메모리 size 줄이기
			auto iter = m_memList.end();
			--iter;
			if (!MarkCheck(m_memList.size() - 1))
			{
				//MarkOff(m_memList.back());
				free(m_memList.back());
				m_memList.back() = nullptr;
				m_memList.erase(iter);

				// memsize 줄이기
				m_memSize >>= 1;

				// map 줄이기
				size_t i = ConvertMemSizeToVecIndex();
				while (true)
				{
					auto iter = m_freeIndex.find(i);
					if (iter == m_freeIndex.end())
					{
						break;
					}
					m_freeIndex.erase(iter);
				}
			}
		}
	}

	BYTE * Malloc()
	{
		do 
		{
			if (!m_freeIndex.empty())
			{
				auto iter = m_freeIndex.begin();
				//auto vectPtr = NonMark(m_memList[iter->first]);
				auto vectPtr = m_memList[iter->first];
				auto retPtr = reinterpret_cast<BYTE *>(vectPtr[iter->second * m_blockSize]);
				//MarkOn(m_memList[iter->first]);

				m_usingPage.insert(std::make_pair(retPtr, std::make_pair(iter->first, iter->second)));
				m_freeIndex.erase(iter);

				return retPtr;
			}

			// 할당할 메모리 공간이 없다
			// 메모리 size 늘이기
			m_memList.emplace_back(reinterpret_cast<BYTE*>(malloc(m_blockSize * m_memSize)));
			m_memSize *= 2;
			size_t vecIndex = ConvertMemSizeToVecIndex() - 1;
			for (size_t i = 0; i < (m_memSize / 2); ++i)
			{
				m_freeIndex.insert(std::make_pair(vecIndex, i));
			}

		} while (true);
	}

private:
	const size_t m_blockSize{ 0 };
	size_t m_memSize{ 0 };
	std::multimap<size_t, size_t> m_freeIndex;
	std::unordered_map<BYTE *, Spot> m_usingPage;
	std::vector<BYTE *> m_memList;

	void MarkOn(BYTE * ptr)
	{
		ptr = reinterpret_cast<BYTE *>(reinterpret_cast<long>(ptr) | 1L);
	}

	void MarkOff(BYTE * ptr)
	{
		ptr = reinterpret_cast<BYTE *>(reinterpret_cast<long>(ptr) & -2L);
	}

	const bool MarkCheck(const size_t & i)
	{
		size_t cnt{ 0 };

		auto iter = m_freeIndex.find(i);

		while (iter != m_freeIndex.end()
			&& iter->first == i)
		{
			++cnt;
			++iter;
		}

		if (cnt == (m_memSize / 2))
		{
			return true;
		}
		return false;
	}

	const bool MarkCheck(BYTE * ptr)
	{
		if (reinterpret_cast<long>(ptr) & 1)
		{
			return true;
		}

		return false;
	}

	const BYTE * NonMark(const BYTE * ptr) const
	{
		return reinterpret_cast<BYTE *>(reinterpret_cast<long>(ptr) & -2L);
	}

	const size_t ConvertMemSizeToVecIndex()
	{
		size_t i = m_memSize;
		size_t cnt{ 0 };

		do 
		{
			++cnt;
		} while (i >>= 1);

		return cnt;
	}
};

class MyMemPool
{
public:
	MyMemPool();
	~MyMemPool();
};

