#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

/**
 * @brief 通用环形缓冲模板类
 * @tparam T  缓冲区中存储的数据类型（例如 MainCirDataRaw::Sample 或自定义结构体）
 * @tparam N  缓冲区长度（必须为2的幂，方便取模运算优化）
 *
 * @note
 *  - 适合高频数据采样 + 低频消费的生产者-消费者模型。
 *  - 无锁写入，适用于单生产者（高优先级任务）+ 单消费者（低优先级任务）。
 *  - 若需要多消费者，可在外部增加只读快照或复制机制。
 */
template <typename T, size_t N>
class RingBuffer {
  public:
    static_assert((N & (N - 1)) == 0, "N must be power of two for fast modulo.");

    RingBuffer() = default;
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    /**
     * @brief 获取当前可写入的缓冲单元引用。
     * @return 对应缓冲单元的引用。
     */
    inline T& currentWriteSlot() noexcept {
        return buffer_[write_idx_];
    }

    /**
     * @brief 无锁写入一帧，成功返回 true，满则返回 false
     */
    inline bool tryPush(const T& v) noexcept {
        const size_t next = (write_idx_ + 1) & (N - 1);
        if (next == read_idx_) {
            return false; // full
        }
        buffer_[write_idx_] = v;
        write_idx_ = next;
        return true;
    }

    /**
     * @brief 满时丢弃最旧数据后写入（“只要最新”场景）
     */
    inline void pushOverwrite(const T& v) noexcept {
        const size_t next = (write_idx_ + 1) & (N - 1);
        if (next == read_idx_) {
            // drop oldest
            read_idx_ = (read_idx_ + 1) & (N - 1);
        }
        buffer_[write_idx_] = v;
        write_idx_ = next;
    }

    /**
     * @brief 写入完成后推进写指针。
     */
    inline void advanceWrite() noexcept {
        write_idx_ = (write_idx_ + 1) & (N - 1);
    }

    /**
     * @brief 检查是否存在未读取的新数据。
     */
    inline bool hasNewData() const noexcept {
        return read_idx_ != write_idx_;
    }

    /**
     * @brief 读取一帧数据并推进读指针。
     * @return 数据副本（为了安全性）
     */
    inline T read() noexcept {
        T data = buffer_[read_idx_];
        read_idx_ = (read_idx_ + 1) & (N - 1);
        return data;
    }

    /**
     * @brief 无锁尝试读取一帧，成功返回 true。
     */
    inline bool tryPop(T& out) noexcept {
        if (read_idx_ == write_idx_) {
            return false;
        }
        out = buffer_[read_idx_];
        read_idx_ = (read_idx_ + 1) & (N - 1);
        return true;
    }

    /**
     * @brief 直接读取缓冲区引用（不推进指针）。
     * @note  使用时需确保数据未被覆盖。
     */
    inline const T& peek() const noexcept {
        return buffer_[read_idx_];
    }

    /**
     * @brief 跳过当前帧（丢弃未处理数据）。
     */
    inline void skip() noexcept {
        read_idx_ = (read_idx_ + 1) & (N - 1);
    }

    /**
     * @brief 获取当前环形缓冲容量。
     */
    static constexpr size_t capacity() noexcept { return N; }

    /**
     * @brief 获取当前缓冲区中的数量 
     * 
     */
    inline size_t num() const noexcept {
        if (write_idx_ >= read_idx_)
            return write_idx_ - read_idx_;
        else
            return N - (read_idx_ - write_idx_);
    }

    inline size_t readIndex() { return read_idx_; }

    inline size_t writeIndex() { return write_idx_; }

  private:
    std::array<T, N> buffer_{};     ///< 存储数据的环形数组
    volatile size_t write_idx_ = 0; ///< 写指针
    volatile size_t read_idx_ = 0;  ///< 读指针
};
