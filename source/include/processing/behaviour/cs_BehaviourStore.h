/*
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Sep 23, 2019
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#pragma once

#include <events/cs_EventListener.h>
#include <processing/behaviour/cs_Behaviour.h>

#include <array>
#include <optional>
#include <vector>

/**
 * Keeps track of the behaviours that are active on this crownstone.
 */
class BehaviourStore : public EventListener {
    private:
    static constexpr size_t MaxBehaviours = 10;
    static std::array<std::optional<Behaviour>,MaxBehaviours> activeBehaviours;
    
    public:
    /**
     * handles events concerning updates of the active behaviours on this crownstone.
     */
    virtual void handleEvent(event_t& evt);

    /**
     * Stores the given behaviour [b] at given [index] in the activeBehaviours array.
     * 
     * Note: currently doesn't persist states.
     * 
     * Returns true on success, false if [index] is out of range.
     */
    bool saveBehaviour(Behaviour b, uint8_t index);

    /**
     * Remove the behaviour at [index]. If [index] is out of bounds,
     * or no behaviour exists at [index], false is returned. Else, true.
     */
    bool removeBehaviour(uint8_t index);

    static inline const std::array<std::optional<Behaviour>,MaxBehaviours>& getActiveBehaviours() {
        return activeBehaviours;
    }

    private:

    class InterfaceB {
        /**
         * Returns an index in range [0,MaxBehaviours) on succes, 
         * or 0xffffffff if it couldn't be saved.
         */
         uint8_t save(Behaviour b);

        /**
         * Replace the behaviour at [index] with [b]
         * postcondition is identical to the
         * postcondition of calling save(b) when it returns [index].
         */
        bool replace(uint8_t index, Behaviour b);

        /**
         * deletes the behaviour at [index] the behaviour is removed from storage.
         */
        bool remove(uint8_t index);

        /**
         *  returns the stored behaviour at [index].
         */
        Behaviour get(uint8_t index);

        /**
         * returns a map with the currently occupied indices and the 
         * behaviours at those indices.
         */
        std::vector<std::pair<uint8_t,Behaviour>> get();

        /**
         * returns the hash of the behaviour at [index]. If no behaviour
         * is stored at this index, 0xffffffff is returned.
         */
        uint32_t hash(uint8_t index);

        /**
         * returns a hash value that takes all state indices into account.
         * this value is expected to change after any call to update/save/remove.
         * 
         * A (phone) application can compute this value locally given the set of 
         * index/behaviour pairs it expects to be present on the Crownstone.
         * Checking if this differs from the one received in the crownstone state message
         * enables the application to resync.
         */
        uint32_t hash();

    } interfaceB;
};