Date do(
	/*doc Date today 
	Set the receiver to the current date, no time information
	is included. See `now' for the current date and time.
	*/
	
	today := method(Date now setHour(0) setMinute(0) setSecond(0))

	//doc Date isToday Returns true if the receiver's date is today's date.
	isToday := method(
		now := Date clone now
		now year == year and now month == month and now day == day
	)

	//doc Date secondsToRun(expression) Evaluates message and returns a Number whose value is the number of seconds taken to do the evaluation
	secondsToRun := method(
		t1 := Date clone now
		call relayStopStatus(call evalArgAt(0))
		dt := Date clone now secondsSince(t1)
	)

	//doc Date asAtomDate Returns the date formatted as a valid atom date (rfc4287) in the system's timezone.
	Date asAtomDate := method(
		asString("%Y-%m-%dT%H:%M:%S") .. gmtOffset asMutable atInsertSeq(3, ":")
	)
			
	justSerialized := method(stream,
		stream write("Date clone do(",
			"setYear(", self year, ") ",
			"setMonth(", self month, ") ",
			"setDay(", self day, ") ",
			"setHour(", self hour, ") ",
			"setMinute(", self minute, ") ",
			"setSecond(", self second, ")",
			");")
	)
)

Number do(
	/*doc Number years Returns Duration of receiver's years.

Example:
<code>
Io> 1 years
==> 1 years 00 days 00:00:0.000000
Io> 20 years
==> 20 years 00 days 00:00:0.000000
</code>

With this, you can do things such as:
<code>
Io> Date clone now + 5 years
==> 2011-11-14 18:44:33 EST
Io> Date clone now + 2 years + 3 days + 22 minutes
==> 2008-11-17 19:06:54 EST
</code>
*/
	years := method(Duration clone setYears(self))

	//doc Number days Returns Duration of receiver's days. See `years' for a few examples.
	days := method(Duration clone setDays(self))

	//doc Number hours Returns Duration of receiver's hours. See `years' for a few examples.
	hours := method(Duration clone setHours(self))

	//doc Number minutes Returns Duration of receiver's minutes. See `years' for a few examples.
	minutes := method(Duration clone setMinutes(self))

	//doc Number seconds Returns Duration of receiver's seconds. See `years' for a few examples.
	seconds := method(Duration clone setSeconds(self))
)

Duration do(
	//doc Duration + Returns a new Duration of the two added.
	setSlot("+", method(d, self clone += d))

	//doc Duration - Returns a new Duration of the two subtracted.
	setSlot("-", method(d, self clone -= d))
)
