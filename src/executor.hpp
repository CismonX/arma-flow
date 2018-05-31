//
// arma-flow/executor.hpp
//
// @author CismonX
//

#pragma once

namespace flow
{
    /// Forward declaration.
    class factory;

    /// Controls the execution of this program.
    class executor
    {
        /// The factory instance.
        factory* factory_;

    public:
        /**
         * Default constructor.
         */
        explicit executor();

        /// Do execute.
        void execute(int argc, char** argv) const;
    };
}