#include <juce_audio_basics/juce_audio_basics.h>
#include <cstdlib>
#include <cstdio>

int main()
{
    juce::UnitTestRunner runner;
    runner.runAllTests();

    bool anyFailures = false;

    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        auto* result = runner.getResult(i);
        std::printf("%s: %d passes, %d failures\n",
                    (result->unitTestName + " / " + result->subcategoryName).toRawUTF8(),
                    result->passes, result->failures);

        if (result->failures > 0)
            anyFailures = true;
    }

    return anyFailures ? EXIT_FAILURE : EXIT_SUCCESS;
}
