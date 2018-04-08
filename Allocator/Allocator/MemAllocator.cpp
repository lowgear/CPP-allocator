#include <exception>

namespace talloc
{
	typedef size_t PtrBase;

	struct Chunk
	{
		Chunk* next;
		PtrBase level;

		Chunk(Chunk* next, PtrBase level)
		{
			this->level = level;
			this->next = next;
		}
	};

	class invalid_state_error : std::exception
	{
	public:
		explicit invalid_state_error(char const* m) : std::exception(m)
		{
		}
	};

	class MemAllocator
	{
	private:
		Chunk* buffer;
		Chunk** freeLists;

		PtrBase bufferLength;
		PtrBase bufferDepth;
		static PtrBase const CHUNK_SIZE = sizeof(Chunk);

		static PtrBase chunkCapacity(PtrBase level)
		{
			return (CHUNK_SIZE << level) - sizeof(PtrBase);
		}

		void fragment(PtrBase const level) const
		{
			Chunk* left = freeLists[level];
			Chunk* right = freeLists[level] + (1 << (level - 1));

			freeLists[level] = freeLists[level]->next;

			*left = Chunk(right, level - 1);
			*right = Chunk(freeLists[level - 1], level - 1);

			freeLists[level - 1] = left;
		}

		void insertIntoFree(Chunk* chunk)
		{
			while (chunk->level < bufferDepth)
			{
				Chunk* const pair = buffer + ((chunk - buffer) ^ (0b1 << chunk->level));
				Chunk** current = freeLists + chunk->level;
				while (*current && *current < pair)
					current = &((*current)->next);

				if (*current != pair)
				{
					chunk->next = *current;
					*current = chunk;
					break;
				}

				*current = pair->next;
				chunk = chunk < pair ? chunk : pair;
				chunk->level += 1;
			}
		}

	public:
		MemAllocator(PtrBase* const buffer, PtrBase const buffer_length)
		{
			bufferDepth = 1;
			while ((CHUNK_SIZE << bufferDepth) + sizeof(PtrBase*) * (bufferDepth + 1) <= buffer_length)
				++bufferDepth;

			freeLists = reinterpret_cast<Chunk**>(buffer);
			this->buffer = reinterpret_cast<Chunk*>(buffer + bufferDepth);
			this->bufferLength = CHUNK_SIZE << (bufferDepth - 1);

			for (PtrBase i = 0; i < bufferDepth; ++i)
				freeLists[i] = nullptr;

			freeLists[bufferDepth - 1] = this->buffer;
			this->buffer[0] = Chunk(nullptr, bufferDepth - 1);
		}

		PtrBase* alloc(PtrBase const size)
		{
			PtrBase level = 0;
			while (level < bufferDepth
				&& (chunkCapacity(level) < size
					|| freeLists[level] == nullptr))
				level++;

			if (level >= bufferDepth)
				throw invalid_state_error("not enough buffer memory or buffer is fragmented");

			while (level > 0 && chunkCapacity(level - 1) >= size)
			{
				fragment(level);
				--level;
			}

			PtrBase* chunk = reinterpret_cast<PtrBase*>(freeLists[level]);
			freeLists[level] = freeLists[level]->next;
			*chunk = level;

			return chunk + 1;
		}

		void free(PtrBase* ptr)
		{
			PtrBase level = *(ptr - 1);
			Chunk* chunk = reinterpret_cast<Chunk*>(ptr - 1);

			*chunk = Chunk(nullptr, level);
			insertIntoFree(chunk);
		}
	};
}
