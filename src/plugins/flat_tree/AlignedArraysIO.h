// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

namespace flatio {

    class AlignedArraysIO {
    public:

        // The function is to bind the class to TTree (set branches)
        virtual void bindToTree(TTree *tree) = 0;

        // Clear before each event
        virtual void clear() = 0;
    };

} // flatio
