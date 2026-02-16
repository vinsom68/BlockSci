//
//  witness_unknown_script.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 10/22/18.
//

#include <blocksci/scripts/witness_unknown_script.hpp>

#include "bitcoin_segwit_addr.hpp"

#include <internal/address_info.hpp>
#include <internal/data_access.hpp>
#include <internal/script_access.hpp>
#include <internal/script_view.hpp>

#include <range/v3/view/split.hpp>

#include <sstream>

namespace blocksci {

    ScriptAddress<AddressType::WITNESS_UNKNOWN>::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptAddress(addressNum_, access_.getScripts().getScriptData<dedupType(addressType)>(addressNum_), access_) {}
    
    uint8_t ScriptAddress<AddressType::WITNESS_UNKNOWN>::witnessVersion() const {
        return getData()->witnessVersion;
    }
    
    ranges::optional<ranges::any_view<ranges::any_view<unsigned char>>> ScriptAddress<AddressType::WITNESS_UNKNOWN>::getWitnessStack() const {
        if (rawInputData != nullptr) {
            return rawInputData->scriptData | ranges::views::split(0xfe);
        } else {
            return ranges::nullopt;
        }
    }
    
    CScriptView ScriptAddress<AddressType::WITNESS_UNKNOWN>::getWitnessScript() const {
        return {getData()->scriptData.begin(), getData()->scriptData.end()};
    }
    
    std::string ScriptAddress<AddressType::WITNESS_UNKNOWN>::getWitnessScriptString() const {
        return ScriptToAsmStr(getWitnessScript());
    }

    std::string ScriptAddress<AddressType::WITNESS_UNKNOWN>::addressString() const {
        std::vector<uint8_t> witprog;
        witprog.insert(witprog.end(), getData()->scriptData.begin(), getData()->scriptData.end());
        return segwit_addr::encode(getAccess().config.chainConfig, witnessVersion(), witprog);
    }
    
    std::string ScriptAddress<AddressType::WITNESS_UNKNOWN>::toString() const {
        std::stringstream ss;
        auto address = addressString();
        if (address.empty()) {
            ss << "WitnessUnknownScript()";
        } else {
            ss << "WitnessUnknownAddress(" << address << ")";
        }
        return ss.str();
    }
    
    std::string ScriptAddress<AddressType::WITNESS_UNKNOWN>::toPrettyString() const {
        std::stringstream ss;
        auto address = addressString();
        if (address.empty()) {
            ss << "WitnessUnknownScript()";
        } else {
            ss << "WitnessUnknownAddress(" << address << ", witness_version=" << int(witnessVersion()) << ")";
        }
        return ss.str();
    }
} // namespace blocksci
