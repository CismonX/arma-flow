#pragma once


namespace flow
{
    class executor
    {
    public:
        explicit executor() = default;

        static void print_and_exit(const std::string& message);

        void execute(int argc, char** argv) const;
    };
}