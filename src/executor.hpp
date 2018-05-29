//
// arma-flow/executor.hpp
//
// @author CismonX
//

#pragma once

namespace flow
{
    /// Controls the execution of this program.
    class executor
    {
    public:
        /**
         * Default constructor.
         */
        explicit executor() = default;

        /// Do execute.
        void execute(int argc, char** argv) const;
    };
}