//
//  algorithms.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/18/17.
//

#ifndef chain_algorithms_hpp
#define chain_algorithms_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/transaction.hpp>

#include <range/v3/range_for.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/view/transform.hpp>

namespace blocksci {
    template <typename B, typename = void>
    struct range_value_is_output_pointer : std::false_type {};

    template <typename B>
    struct range_value_is_output_pointer<B, std::void_t<ranges::range_value_t<B>>>
        : std::is_same<ranges::range_value_t<B>, OutputPointer> {};

    template <typename B, typename = void>
    struct range_value_is_input : std::false_type {};

    template <typename B>
    struct range_value_is_input<B, std::void_t<ranges::range_value_t<B>>>
        : std::is_same<ranges::range_value_t<B>, Input> {};

    template <typename B, typename = void>
    struct range_value_is_output : std::false_type {};

    template <typename B>
    struct range_value_is_output<B, std::void_t<ranges::range_value_t<B>>>
        : std::is_same<ranges::range_value_t<B>, Output> {};

    template <typename B, typename = void>
    struct range_value_is_tx : std::false_type {};

    template <typename B>
    struct range_value_is_tx<B, std::void_t<ranges::range_value_t<B>>>
        : std::is_same<ranges::range_value_t<B>, Transaction> {};

    template <typename B, typename = void>
    struct range_value_is_block : std::false_type {};

    template <typename B>
    struct range_value_is_block<B, std::void_t<ranges::range_value_t<B>>>
        : std::is_same<ranges::range_value_t<B>, Block> {};

    template <typename B, typename = void>
    struct range_value_is_optional_input : std::false_type {};

    template <typename B>
    struct range_value_is_optional_input<B, std::void_t<ranges::range_value_t<B>>>
        : std::is_same<ranges::range_value_t<B>, ranges::optional<Input>> {};

    template <typename B, typename = void>
    struct range_value_is_optional_output : std::false_type {};

    template <typename B>
    struct range_value_is_optional_output<B, std::void_t<ranges::range_value_t<B>>>
        : std::is_same<ranges::range_value_t<B>, ranges::optional<Output>> {};

    template <typename B, typename = void>
    struct range_value_is_optional_tx : std::false_type {};

    template <typename B>
    struct range_value_is_optional_tx<B, std::void_t<ranges::range_value_t<B>>>
        : std::is_same<ranges::range_value_t<B>, ranges::optional<Transaction>> {};

    template <typename B, typename = void>
    struct range_value_is_optional_block : std::false_type {};

    template <typename B>
    struct range_value_is_optional_block<B, std::void_t<ranges::range_value_t<B>>>
        : std::is_same<ranges::range_value_t<B>, ranges::optional<Block>> {};
    
    template <typename B>
    inline constexpr bool isOutputPointerRange =
        ranges::range<B> && range_value_is_output_pointer<B>::value;
    
    template <typename B>
    inline constexpr bool isInputRange =
        ranges::range<B> && range_value_is_input<B>::value;
    
    template <typename B>
    inline constexpr bool isOutputRange =
        ranges::range<B> && range_value_is_output<B>::value;
    
    template <typename B>
    inline constexpr bool isTxRange =
        ranges::range<B> && range_value_is_tx<B>::value;
    
    template <typename B>
    inline constexpr bool isBlockRange =
        ranges::range<B> && range_value_is_block<B>::value;
    
    template <typename B>
    CPP_concept_bool isTx = std::is_same<B, Transaction>::value;
    
    template <typename B>
    CPP_concept_bool isBlockchain = std::is_same<B, Blockchain>::value;
    
    template <typename B>
    inline constexpr bool isOptionalInputRange =
        ranges::range<B> && range_value_is_optional_input<B>::value;
    
    template <typename B>
    inline constexpr bool isOptionalOutputRange =
        ranges::range<B> && range_value_is_optional_output<B>::value;
    
    template <typename B>
    inline constexpr bool isOptionalTxRange =
        ranges::range<B> && range_value_is_optional_tx<B>::value;
    
    template <typename B>
    inline constexpr bool isOptionalBlockRange =
        ranges::range<B> && range_value_is_optional_block<B>::value;
    
    template<typename T>
    struct fail_helper : std::false_type
    { };
    
    CPP_template(typename B)(requires isTxRange<B>)
    inline auto BLOCKSCI_EXPORT txes(B && b) {
        return std::forward<B>(b);
    }
    
    CPP_template(typename B)(requires isBlockRange<B>)
    inline auto BLOCKSCI_EXPORT txes(B && b) {
        return std::forward<B>(b) | ranges::views::join;
    }
    
    inline auto BLOCKSCI_EXPORT inputs(const Transaction &tx) {
        return tx.inputs();
    }
    
    inline auto BLOCKSCI_EXPORT inputs(Transaction &tx) {
        return tx.inputs();
    }
    
    CPP_template(typename B)(requires isInputRange<B>)
    inline auto BLOCKSCI_EXPORT inputs(B && b) {
        return std::forward<B>(b);
    }
    
    CPP_template(typename B)(requires isTxRange<B> || isBlockRange<B>)
    inline auto BLOCKSCI_EXPORT inputs(B && b) {
        return txes(std::forward<B>(b)) | ranges::views::transform([](const Transaction &tx) { return tx.inputs(); }) | ranges::views::join;
    }
    
    inline auto BLOCKSCI_EXPORT outputs(const Transaction &tx) {
        return tx.outputs();
    }
    
    inline auto BLOCKSCI_EXPORT outputs(Transaction &tx) {
        return tx.outputs();
    }
    
    CPP_template(typename B)(requires isOutputRange<B>)
    inline auto BLOCKSCI_EXPORT outputs(B && b) {
        return std::forward<B>(b);
    }
    
    CPP_template(typename B)(requires isTxRange<B> || isBlockRange<B>)
    inline auto BLOCKSCI_EXPORT outputs(B && b) {
        return txes(std::forward<B>(b)) | ranges::views::transform([](const Transaction &tx) { return tx.outputs(); }) | ranges::views::join;
    }
    
    CPP_template(typename B)(requires isOutputPointerRange<B>)
    inline auto BLOCKSCI_EXPORT outputs(B && b, DataAccess &access) {
        return std::forward<B>(b) | ranges::views::transform([&access](const OutputPointer &pointer) { return Output(pointer, access); });
    }
    
    template <typename T>
    inline auto BLOCKSCI_EXPORT outputsUnspent(T && t) {
        return outputs(std::forward<T>(t)) | ranges::views::remove_if([](const Output &output) { return output.isSpent(); });
    }
    
    template <typename T>
    inline auto BLOCKSCI_EXPORT outputsSpentBeforeHeight(T && t, blocksci::BlockHeight blockHeight) {
        return outputs(std::forward<T>(t)) | ranges::views::filter([=](const Output &output) { return output.isSpent() && output.getSpendingTx()->getBlockHeight() < blockHeight; });
    }
    
    template <typename T>
    inline auto BLOCKSCI_EXPORT outputsSpentAfterHeight(T && t, blocksci::BlockHeight blockHeight) {
        return outputs(std::forward<T>(t)) | ranges::views::filter([=](const Output &output) { return output.isSpent() && output.getSpendingTx()->getBlockHeight() >= blockHeight; });
    }
    
    template <typename T>
    inline auto BLOCKSCI_EXPORT inputsCreatedAfterHeight(T && t, blocksci::BlockHeight blockHeight) {
        return inputs(std::forward<T>(t)) | ranges::views::filter([=](const Input &input) { return input.getSpentTx().getBlockHeight() >= blockHeight; });
    }
    
    template <typename T>
    inline auto BLOCKSCI_EXPORT inputsCreatedBeforeHeight(T && t, blocksci::BlockHeight blockHeight) {
        return inputs(std::forward<T>(t)) | ranges::views::filter([=](const Input &input) { return input.getSpentTx().getBlockHeight() < blockHeight; });
    }

    template <typename T>
    inline auto BLOCKSCI_EXPORT outputsSpentWithinRelativeHeight(T && t, blocksci::BlockHeight difference) {
        return outputs(std::forward<T>(t)) | ranges::views::filter([=](const Output &output) {
            return output.isSpent() && output.getSpendingTx()->getBlockHeight() - output.getBlockHeight() < difference;
        });
    }
    
    template <typename T>
    inline auto BLOCKSCI_EXPORT outputsSpentOutsideRelativeHeight(T && t, blocksci::BlockHeight difference) {
        return outputs(std::forward<T>(t)) | ranges::views::filter([=](const Output &output) {
            return output.isSpent() && output.getSpendingTx()->getBlockHeight() - output.getBlockHeight() >= difference;
        });
    }
    
    template <typename T>
    inline auto BLOCKSCI_EXPORT inputsCreatedWithinRelativeHeight(T && t, blocksci::BlockHeight difference) {
        return inputs(std::forward<T>(t)) | ranges::views::filter([=](const Input &input) {
            return input.blockHeight - input.getSpentTx().getBlockHeight() < difference;
        });
    }
    
    template <typename T>
    inline auto BLOCKSCI_EXPORT inputsCreatedOutsideRelativeHeight(T && t, blocksci::BlockHeight difference) {
        return inputs(std::forward<T>(t)) | ranges::views::filter([=](const Input &input) {
            return input.blockHeight - input.getSpentTx().getBlockHeight() >= difference;
        });
    }
    
    template <typename T>
    inline auto BLOCKSCI_EXPORT outputsOfType(T && t, AddressType::Enum type) {
        return outputs(std::forward<T>(t)) | ranges::views::filter([=](const Output &output) { return output.getType() == type; });
    }
    
    template <typename T>
    inline auto BLOCKSCI_EXPORT inputsOfType(T && t, AddressType::Enum type) {
        return inputs(std::forward<T>(t)) | ranges::views::filter([=](const Input &input) { return input.getType() == type; });
    }
    
    template <typename T>
    inline uint64_t BLOCKSCI_EXPORT inputCount(T && t) {
        auto values = txes(std::forward<T>(t)) | ranges::views::transform([](const Transaction &tx) { return tx.inputCount(); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    template <typename T>
    inline uint64_t BLOCKSCI_EXPORT outputCount(T && t) {
        auto values = txes(std::forward<T>(t)) | ranges::views::transform([](const Transaction &tx) { return tx.outputCount(); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    template <typename T>
    inline int64_t BLOCKSCI_EXPORT totalInputValue(T && t) {
        auto values = inputs(std::forward<T>(t)) | ranges::views::transform([](const Input &a) { return a.getValue(); });
        return ranges::accumulate(values, int64_t{0});
    }
    
    template <typename T>
    inline int64_t BLOCKSCI_EXPORT totalOutputValue(T && t) {
        auto values = outputs(std::forward<T>(t)) | ranges::views::transform([](const Output &a) { return a.getValue(); });
        return ranges::accumulate(values, int64_t{0});
    }
    
    inline int64_t BLOCKSCI_EXPORT fee(const Transaction &tx) {
        return tx.fee();
    }

    /** Calculate the total balance of a collection of outputs, optionally only up to a given block height */
    template <typename T>
    inline int64_t BLOCKSCI_EXPORT balance(BlockHeight height, T && t) {
        int64_t value = 0;
        if (height == -1) {
            RANGES_FOR(auto output, t) {
                if (!output.isSpent()) {
                    value += output.getValue();
                }
            }
        } else {
            RANGES_FOR(auto output, t) {
                if (output.getBlockHeight() <= height && (!output.isSpent() || output.getSpendingTx()->getBlockHeight() > height)) {
                    value += output.getValue();
                }
            }
        }
        return value;
    }

    template <typename T>
    inline auto BLOCKSCI_EXPORT feeLessThan(T &t, int64_t value) {
        return txes(t) | ranges::views::filter([=](const Transaction &tx) {
            return fee(tx) < value;
        });
    }

    template <typename T>
    inline auto BLOCKSCI_EXPORT feeGreaterThan(T &t, int64_t value) {
        return txes(t) | ranges::views::filter([=](const Transaction &tx) {
            return fee(tx) > value;
        });
    }
    
    template <typename T>
    inline auto BLOCKSCI_EXPORT fees(T &t) {
        return ranges::views::transform(txes(t), [](Transaction && tx) {
            return fee(tx);
        });
    }
    
    inline double BLOCKSCI_EXPORT feePerByte(const Transaction &tx) {
        return static_cast<double>(fee(tx)) / static_cast<double>(tx.sizeBytes());
    }
    
    template <typename T>
    inline auto BLOCKSCI_EXPORT feesPerByte(T &t) {
        return ranges::views::transform(txes(t), feePerByte);
    }
    
    template <typename T>
    inline int64_t BLOCKSCI_EXPORT totalFee(T &t) {
        return ranges::accumulate(fees(t), int64_t{0});
    }
} // namespace blocksci

#endif /* chain_algorithms_hpp */
