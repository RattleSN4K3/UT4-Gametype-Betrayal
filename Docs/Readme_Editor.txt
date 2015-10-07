=============================================================================
 Betrayal for UT4 (Editor)
 October 7, 2015
 Version: 0.1.4
 ----------------------------------------------------------------------------
 Cooperate to get bonus points. Betray your team to keep them.
=============================================================================
 Game: Unreal Tournament (UE4)
 Size: ~ 0.496 - 9.55 MB
 Version: 0.1.4
 Compatibility: Build 2710036 (10/1/2015)
 Type: C++
 Credits: Epic Games
-----------------------------------------------------------------------------
 Coded by RattleSN4K3
 Mail: RattleSN4K3@gmail.com
=============================================================================


Description:
--------------------------------------------------------
Betrayal is a new instagib game type that pits players against each other while
they cooperate in temporary alliances. Only one player is the final winner, and
the difference between victory and defeat often lies in knowing when to betray
your teammates.

A score above opponents' heads reflect their value if you gib them using the
primary fire of your instagib rifle. Current teammates are blue, and every kill
by a teammate adds one point to the team pot, which is shown at the top of your
HUD. Teammates can betray each other using the alternate fire of the rifle, which
shoots a blue beam. Assassinating a teammate gives the assassin both the score
over that player's head, plus all the points accrued in the team pot.  

However, the assassin becomes a Rogue for 30 seconds, during which he is not on
any team, and his former teammates can garner a score bonus for achieving
retribution by killing him. The score above a Rogue's head is red for his former
teammates.Once the rogue timer has counted down, the rogue will automatically
join a new team when teammates are available.
  
Daggers beside each player's name shows the number of times that player has
betrayed his teammates.  Watch your back around teammates; you'll often see
players pause and check each other out as they enter a room together.

Sometimes, players may try to goad a teammate into killing them by shooting blue
beams by their ear when the team pot is small, hoping to later cash in on the
retribution bonus.  As the players near victory, they'll often try to time a
final betrayal to send them over the top.


Features:
--------------------------------------------------------
- Instagib only gametype
- Semi team play
- Custom sounds and textures (ported from UT3)
- Player beacons including player score
- Bot AI betraying team mates
- Scoreboard showing daggers (and other stats)
- HUD showing game type info like team, current pot and roque players
- Additional stats for player cards



Installation:
--------------------------------------------------------
- Extract the zip file (copy the content) to your Unreal Tournament Editor folder:  
  UnrealTournamentEditor\

Manually:
- Copy files/folders from "Binaries\" to
  UnrealTournamentEditor\Binaries
- Copy files/folders from "Content\" to
  UnrealTournamentEditor\Content



Usage:
--------------------------------------------------------
- Start the editor
- Open a map
- Choose the game type "UTBetrayalGameMode"
- Play the map
- Enjoy



Changelog:
--------------------------------------------------------
v0.1.4
- Changed: Minor code improvements

v0.1.3:
- Changed: No teams with less players than 3
- Changed: Don't apply team color on dedicated servers
- Fixed: Bots not shooting at team mates if there are no other players
- Fixed: Team color not changing when teams are dissolved

v0.1.2:
- Changed: Updated code for latest build
- Changed: Removed Force-respawn option from game options
- Changed: Betrayal tab in player cards removed and integrated into Score tab (temporarily)
- Changed: Repositioned kill feed messages
- Added: Stat counting for weapon alt kills, alt deaths, hits and shots
- Fixed: Team color not set and replicated properly
- Fixed: Team color is removed on damage (falling, etc.)
- Fixed: PostRender beacon does not work reliable
- Fixed: Muzzle flash for InstaGib rifle
- Fixed: Missing hand animation for InstaGib rifle
- Fixed: Possible memory leak in hub sessions

v0.1.1
- Added: Linux support
- Fixed: Compiling for Linux using CLANG
