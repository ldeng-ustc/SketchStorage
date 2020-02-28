all: index.html schedule.html MENU mysite.conf

index.html: index.jemdoc
	jemdoc index.jemdoc

schedule.html: schedule.jemdoc
	jemdoc schedule.jemdoc
