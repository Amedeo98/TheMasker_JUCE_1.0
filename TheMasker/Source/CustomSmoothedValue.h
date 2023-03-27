/*
  ==============================================================================

    CustomSmoothedValue.h
    Created: 25 Mar 2023 1:31:01pm
    Author:  nikid

  ==============================================================================
*/
namespace juce {



    template <typename FloatType, typename SmoothingType = ValueSmoothingTypes::Linear>
    class CustomSmoothedValue : public SmoothedValueBase <SmoothedValue <FloatType, SmoothingType>>
    {
    public:
        //==============================================================================
        /** Constructor. */
        CustomSmoothedValue() noexcept
            : CustomSmoothedValue((FloatType)(std::is_same_v<SmoothingType, ValueSmoothingTypes::Linear> ? 0 : 1))
        {
        }

        /** Constructor. */
        CustomSmoothedValue(FloatType initialValue) noexcept
        {
            // Multiplicative smoothed values cannot ever reach 0!
            jassert(!(std::is_same_v<SmoothingType, ValueSmoothingTypes::Multiplicative> && initialValue == 0));

            // Visual Studio can't handle base class initialisation with CRTP
            this->currentValue = initialValue;
            this->target = this->currentValue;
        }

        //==============================================================================
        /** Reset to a new sample rate and ramp length.
            @param sampleRate           The sample rate
            @param rampLengthInSeconds  The duration of the ramp in seconds
        */
        void reset(double sampleRate, double posRampLengthInSeconds, double negRampLengthInSeconds) noexcept
        {
            jassert(sampleRate > 0 && posRampLengthInSeconds >= 0 && negRampLengthInSeconds >=0);
            reset((int)std::floor(posRampLengthInSeconds * sampleRate),(int)std::floor(negRampLengthInSeconds * sampleRate) );
        }

        /** Set a new ramp length directly in samples.
            @param numSteps     The number of samples over which the ramp should be active
        */
        void reset(int numPosSteps, int numNegSteps) noexcept
        {
            posStepsToTarget = numPosSteps;
            negStepsToTarget = numNegSteps;
            this->setCurrentAndTargetValue(this->target);
        }

        //==============================================================================
        /** Set the next value to ramp towards.
            @param newValue     The new target value
        */
        void setTargetValue(FloatType newValue) noexcept
        {
            isPositive = newValue >= this->currentValue;

            if (newValue == this->target)
                return;


            if (posStepsToTarget <= 0 && negStepsToTarget <= 0)
            {
                this->setCurrentAndTargetValue(newValue);
                return;
            }

            // Multiplicative smoothed values cannot ever reach 0!
            jassert(!(std::is_same_v<SmoothingType, ValueSmoothingTypes::Multiplicative> && newValue == 0));

            this->target = newValue;
            this->countdown = isPositive ? posStepsToTarget : negStepsToTarget;

            setStepSize();
        }

        //==============================================================================
        /** Compute the next value.
            @returns Smoothed value
        */
        FloatType getNextValue() noexcept
        {
            if (!this->isSmoothing())
                return this->target;

            --(this->countdown);

            if (this->isSmoothing())
                setNextValue();
            else
                this->currentValue = this->target;

            return this->currentValue;
        }

        //==============================================================================
        /** Skip the next numSamples samples.
            This is identical to calling getNextValue numSamples times. It returns
            the new current value.
            @see getNextValue
        */
        FloatType skip(int numSamples) noexcept
        {
            if (numSamples >= this->countdown)
            {
                this->setCurrentAndTargetValue(this->target);
                return this->target;
            }

            skipCurrentValue(numSamples);

            this->countdown -= numSamples;
            return this->currentValue;
        }

        //==============================================================================
#ifndef DOXYGEN
 /** Using the new methods:

     lsv.setValue (x, false); -> lsv.setTargetValue (x);
     lsv.setValue (x, true);  -> lsv.setCurrentAndTargetValue (x);

     @param newValue     The new target value
     @param force        If true, the value will be set immediately, bypassing the ramp
 */
        [[deprecated("Use setTargetValue and setCurrentAndTargetValue instead.")]]
        void setValue(FloatType newValue, bool force = false) noexcept
        {
            if (force)
            {
                this->setCurrentAndTargetValue(newValue);
                return;
            }

            setTargetValue(newValue);
        }
#endif

    private:
        //==============================================================================
        template <typename T = SmoothingType>
        void setStepSize() noexcept
        {
            if constexpr (std::is_same_v<T, ValueSmoothingTypes::Linear>)
            {
                step = (this->target - this->currentValue) / (FloatType)this->countdown;
            }
            else if constexpr (std::is_same_v<T, ValueSmoothingTypes::Multiplicative>)
            {
                step = std::exp((std::log(std::abs(this->target)) - std::log(std::abs(this->currentValue))) / (FloatType)this->countdown);
            }
        }

        //==============================================================================
        template <typename T = SmoothingType>
        void setNextValue() noexcept
        {
            if constexpr (std::is_same_v<T, ValueSmoothingTypes::Linear>)
            {
                this->currentValue += step;
            }
            else if constexpr (std::is_same_v<T, ValueSmoothingTypes::Multiplicative>)
            {
                this->currentValue *= step;
            }
        }

        //==============================================================================
        template <typename T = SmoothingType>
        void skipCurrentValue(int numSamples) noexcept
        {
            if constexpr (std::is_same_v<T, ValueSmoothingTypes::Linear>)
            {
                this->currentValue += step * (FloatType)numSamples;
            }
            else if constexpr (std::is_same_v<T, ValueSmoothingTypes::Multiplicative>)
            {
                this->currentValue *= (FloatType)std::pow(step, numSamples);
            }
        }

        //==============================================================================
        FloatType step = FloatType();
        int posStepsToTarget = 0;
        int negStepsToTarget = 0;
        bool isPositive = false;
    };
}
