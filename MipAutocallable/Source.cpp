#include <boost/timer.hpp>
#include <iostream>
#include <ql/quantlib.hpp>
#include <marketdata.hpp>
#include <autocallablesimulation.hpp>

#ifdef BOOST_MSVC
#  include <ql/auto_link.hpp>
#endif

using namespace QuantLib;

// Compute the price of an Autocallable Investment Certificate

int main(int, char*[]) {

	try {

		boost::timer timer;
		std::cout << std::endl;

		//date setting
		Calendar calendar = TARGET();
		DayCounter dayCount = Actual365Fixed();
		Date todaysDate(31, March, 2017);

		Integer fixingDays = 2;
		Date settlementDate = calendar.advance(todaysDate, fixingDays, Days);
		// must be a business day
		settlementDate = calendar.adjust(settlementDate);
		Settings::instance().evaluationDate() = todaysDate;
		todaysDate = Settings::instance().evaluationDate();

		//option input-data		
		Date optionExpiryDate(03, March, 2021);
		Time maturity = dayCount.yearFraction(settlementDate, optionExpiryDate);
		Real strike = 15.08;
		boost::shared_ptr<Quote> underlying(new SimpleQuote(15.35));		

		//discounting curve, dividend curve and volatility term structure
		auto OISTermStructure = MarketData::builddiscountingcurve(settlementDate, fixingDays);
		auto qTermStructure = MarketData::builddividendcurve(settlementDate, fixingDays, OISTermStructure);
		auto varTS = MarketData::buildblackvariancesurface(settlementDate, calendar);
		Volatility sigma = varTS->blackVol(optionExpiryDate, strike);
		const boost::shared_ptr<BlackVolTermStructure> volatility(new BlackConstantVol(settlementDate, calendar, sigma, dayCount));

		//bond discounting curve
		auto bondTermStructure = MarketData::buildbonddiscountingurve(settlementDate, fixingDays);

		//Price calculation via Montecarlo simulation
		AutocallableSimulation autocall(underlying, qTermStructure, bondTermStructure, OISTermStructure, volatility, maturity, strike, settlementDate);
		Size nTimeSteps = 1000;
		Size nSamples = 50000; 
		autocall.compute(nTimeSteps, nSamples);
		
		double seconds = timer.elapsed();
		Integer hours = int(seconds / 3600);
		seconds -= hours * 3600;
		Integer minutes = int(seconds / 60);
		seconds -= minutes * 60;
		std::cout << " \nRun completed in ";
		if (hours > 0)
			std::cout << hours << " h ";
		if (hours > 0 || minutes > 0)
			std::cout << minutes << " m ";
		std::cout << std::fixed << std::setprecision(0)
			<< seconds << " s\n" << std::endl;
			
		return 0;
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "unknown error" << std::endl;
		return 1;
	}
}


