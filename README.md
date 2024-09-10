# AirlineTycoon

This repository aims to complete the partial source code that is provided as a free bonus in the GOG release
of Airline Tycoon Deluxe.

To run it you'll need the game assets from the either the First Class, Evolution or
Deluxe edition of the game. You can purchase these assets from GOG.com: https://www.gog.com/game/airline_tycoon_deluxe

## Major Additions

- Native Linux support
- Dedicated server browser and NAT-punchthrough multiplayer (open source server at: https://github.com/WizzardMaker/ATDMasterServer)

## License

The code in the repository is licensed under the terms included in the GOG release. As such the code can
only be used for non-commercial purposes and remains property of BFG.

It is therefore *not* open-source in the free software sense, for more information refer to the License.txt.

## Building and running

Before building, remember to clone the submodules:
```
git submodule update --init
```

You can see the instructions to build and run the project in [BUILDING.md](BUILDING.md).

## Changes

General:
* Now runnable on Linux
* Allow the game to be ran Windowed / Borderless / Fullscreen

Statistics screen:
* Showing far more categories where money was spent
   * For example income from freight jobs, total tons transported, money spent on planes, sabotage or stocks and money gained from interest, credit or stocks
* Accurate summation of money spent
   * Fixed many bugs where especially money earned / spent by competitor would not show up in balance
* Whether or not values are shown depends on skill of financial advisor (for your airline) or skill of spy (regarding competitors)
* Unlimited statistics: Store statistics data for each day since beginning of the game
* Fix rendering of graph when zooming out
* Fix display of mission target

Information menu:
* Much more information on balance sheet depending on skill of your financial advisor
* New financial summary for quick assessment of the financial health of your airline (e.g. operating profit)
* Multiple balance sheets (for previous day / week / overall)
* More information from spy (e.g. weekly balance and financial summary for each competitor)
* Detailed information from kerosene advisor (quality / value of kerosene, money saved by using tanks)

Option menu:
* Game Speed is adjustable in options menu. Available values: 1, 5, 10, 15, 20, 25, 30 (default value). The hosts game speed is synced to clients in a network game.

Keyboard navigation:
* Allow Enter/Backspace in calculator
* Enable keyboard navigation in Laptop / Globe (arrow keys)
* Enable keyboard navigation in HR folder
    * Flip pages using arrow keys
    * Jump 10/100 pages in HR files using Shift/Ctrl
    * Change salary using +/-
    * Hire/fire using Enter/Backspace
* Enable keyboard navigation in plane prop menu (arrow keys, jump using Shift/Ctrl)
* Adjust route ticket prices in larger steps (using Shift/Ctrl)
* Arrow key navigation for many different menus

Employees:
* More pilots/attendants available for hire
* Slightly increase competence of randomly generated employees
* Generate randomized advisors as well
* Regenerate unemployed employees if not hired within 7 days (prevents buildup of low-skill candidates in long games)
* List automatically sorted by skill
* Update worker happiness based on salary
    * Chance to increase/decrease happiness each day based on how much salary is higher/lower than original salary
* The 10% change when increasing/decreasing salary now always refers to the original salary
* Regularly increase worker happiness if company image is great

Kerosene:
* Adjust impact of bad kerosene:
    * Now depends on ratio of bad kerosene in tank (quadratic function now instead of yes/no)
    * Amount of plane damage due to bad kerosene increased
    * Reasoning: Before, it was very easy to save enormous amounts of money by buying 'bad' kerosene. Now, it is still possible to save money, but you will need to carefully consider how much 'bad' kerosene you put in your tanks (between 10% and 20% can work).
* Kerosene advisor gives hints on how to save money in new kerosene advisor report
* ArabAir offers much larger kerosene tanks
* Do not remember selected kerosene quality for auto purchase (was an undocumented and convoluted 'feature')

Bug fixes:
* Fixed frozen windows on laptop
* Integer overflow fixed when emitting lots of stock (resulted in loosing money when emitting)
* Fixed formula for credit limit
* Stock trading: Show correct new account balance in form (including fee)
* Fix saving/reloading of plane equipment configuration
* Fix bug in gate planning ('no gate' warning despite plenty of free gates available)
* Fix distant rendering of sticky notes in the boss office
* Use correct security measure to protect against route stealing
* Fix calculation of passenger happiness
    * Set passenger rating based on quality + small randomized delta (old code could yield just 'okay' even with high quality)
    * Passengers will tolerate high prices if quality is good
* Fix sabotage that puts leaflets into opponent's plane
    * Now passenger happiness is booked to the statistic of the sabotaging player
* 'Plane crash' sabotage now also affects stock price of victim
* Fix calculation of plane repair cost
    * All cost will show up in plane saldo
    * All cost will show up in plane repair cost total
    * Correctly calculate plane saldo over past 7 days
* Consider also number of first class passengers for statistics
* Do not show route utilization for defeated players
* New sound options (OGG/MIDI) + patched stuttering glich when switching music on Windows 11
* Patched Space station mission prices and texts in stats
* Patched various text scrambling on UI
* Bug fixed in calculation in maximum amount of stock that can be emitted
    * Bug limited max amount of stock to around 2.1 million
    * Intger overflow is fixed now, but the originally intended limit of 250 million was changed to 2.5 million
* Fixed game shifting flights on its own even if they are already locked
    * Could previously cause double-booking of flights (income and cost booked twice)
* Classic mission 04 now uses correct route utilization
    * Previously, even though boss said that routes must be 20% utilized, game would check for 20% plane utilization
* Addon mission 09: Fixed counting of Uhrig flights
    * Note that computer players always have and still are cheating in this mission
* First class mission 07: Only need to have two repaired planes, not all of them in case more than two were bought
* Evolution mission 02: Only need to have five planes with full safety upgrades, not all of them in case more than five were bought
* Fixed many random crashes

AI:
* Uses now same credit limit
* Uses now same rules for trading stock
    * Trading fee (100 + 10% of volume) now also for computer players (fee existed only for player)
    * Do not execute trades in steps of 1000 (this previously made stock prices worse for the human player only)
    * Align function to re-calculate stock price after trade
* Uses now same rules for emitting stock
* Remove sabotage advantages
    * Computer now has to pay for sabotage as well
    * Consider all security measures (e.g. plane crash not possible anymore if plane is protected)
    * Align calculation of arab trust for player and computer
* Remove strange reduction of flight cost in calculation of image change (was a disadvantage for computer player)
* Computer player pays real price for plane upgrades
* Fixed bug that prevented computer players from using routes in most games
    * Computer players will switch to routes in most games eventually
    * Computer players however will use a small cheat that regularly improves their image

Misc:
* Reduce (~ half) cost of plane security upgrades
* Spy reports enemy activity based on skill
* ArabAir opens one hour earlier
* Calculate route utilization as average of previous 7 days
* Adding NOTFAIR cheat to make competitors much richer
* Adding ODDONEOUT cheat to improve image of competitors
* Adding AUTORUN cheat for ultra-fast forward
* Use player colors when showing routes on laptop
* Buy kerosene by clicking price chart
* Change tooltip of savegames (number of days played)
* Company value includes value of kerosene stored in tanks and tanks themselves
* Company value includes value of plane upgrades
* Company value includes value of airline image (money required to reach current image)
* Strikes will start after 9 am now to give player chance to react
* Make planes in main menu comically long
* Decryption of data files with the run argument "/savedata"
* Option "OptionTicketPriceIncrement" to increase ticket price increment per click
* Director's board now allow for more post-it
* Added options "OptionRentOffice*" to customise the branch number available / day.
* Director's board post-it system improved and allow for more cities (up to 7)

## Credits and Contributors

- [WizzardMaker](https://github.com/WizzardMaker) - Main contributor and maintainer
- [CrossVR](https://github.com/CrossVR) - Original contributor
- [mertenpopp](https://github.com/mertenpopp) - Contributor
- [wackfx](https://github.com/wackfx) - Contributor
- [CrazyOrange](https://github.com/CrazyOrange) - Contributor
- [Heftie](https://github.com/Heftie) - Contributor
