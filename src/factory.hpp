//
// arma-flow/factory.hpp
//
// @author CismonX
//

#pragma once

#include "reader.hpp"
#include "args.hpp"
#include "executor.hpp"
#include "calc.hpp"
#include "writer.hpp"

namespace flow
{
    /// Contains factory methods for this program.
    class factory
    {
        /// The reader instance.
        reader reader_;

        /// The arg parser instance.
        args args_;

        /// The executor instance.
        executor executor_;

        /// The power flow calculator instance. 
        calc calc_;

        /// The writer instance.
        writer writer_;

        /// Singleton for this factory.
        static factory singleton_;

        /**
         * Private default constructor.
         */
        explicit factory() = default;

    public:
        /// Get factory.
        static factory* get()
        {
            return &singleton_;
        }

        /// Get reader.
        reader* get_reader()
        {
            return &reader_;
        }

        /// Get arg parser.
        args* get_args()
        {
            return &args_;
        }

        /// Get executor.
        executor* get_executor()
        {
            return &executor_;
        }

        /// Get power flow calculator.
        calc* get_calc()
        {
            return &calc_;
        }

        /// Get writer.
        writer* get_writer()
        {
            return &writer_;
        }
    };

    inline factory factory::singleton_;
}
