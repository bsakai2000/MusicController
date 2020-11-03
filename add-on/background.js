// Connect to the python server
var port = browser.runtime.connectNative("music_controller");

// Global to tell if playing
var is_playing = false;

// The list of sites we want to try, in the order we want
// to try them
var sites = [
	{
		url: "*://play.google.com/music*",
		pausePlay: "document.getElementById('player-bar-play-pause').click()",
		pauseCheck: "document.getElementById('player-bar-play-pause').title == 'Pause'",
		next: "document.getElementById('player-bar-forward').click()",
		prev: "document.getElementById('player-bar-rewind').click()"
	},
	{
		url: "*://music.youtube.com/*",
		pausePlay: "document.getElementById('play-pause-button').click()",
		pauseCheck: "document.getElementById('play-pause-button').title == 'Pause'",
		next: "document.getElementsByClassName('next-button')[0].click()",
		prev: "document.getElementsByClassName('previous-button')[0].click()"
	},
	{
		url: "*://*.youtube.com/*",
		pausePlay: "document.getElementsByClassName('ytp-play-button')[0].click()",
		pauseCheck: "document.getElementsByClassName('ytp-play-button')[0].title == 'Pause (k)'",
		next: "document.getElementsByClassName('ytp-next-button')[0].click()",
		prev: "document.getElementsByClassName('ytp-prev-button')[0].click()"
	},
	{
		url: "*://*.netflix.com/*",
		pausePlay: "Array.from(document.getElementsByClassName('button-nfplayerPause')).concat(Array.from(document.getElementsByClassName('button-nfplayerPlay')))[0].click()",
		pauseCheck: "Array.from(document.getElementsByClassName('button-nfplayerPause')).concat(Array.from(document.getElementsByClassName('button-nfplayerPlay')))[0].getAttribute('aria-label') == 'Pause'",
		next: "document.getElementsByClassName('button-nfplayerFastForward')[0].click()",
		prev: "document.getElementsByClassName('button-nfplayerBackTen')[0].click()"
	},
	{
		url: "*://*.dcuniverse.com/*",
		pausePlay: "document.getElementsByClassName('vjs-play-control')[0].click()",
		pauseCheck: "document.getElementsByClassName('vjs-play-control')[0].title == 'Pause'",
		next: "",
		prev: ""
	},
	{
		url: "*://*.podcasts.google.com/*",
		pausePlay: "document.getElementsByClassName('DPvwYc ERYGad')[0].click()",
		pauseCheck: "document.getElementsByClassName('DPvwYc ERYGad')[0].style.display == 'none'",
		next: "document.getElementsByClassName('DPvwYc UO8XJc')[1].click()",
		prev: "document.getElementsByClassName('DPvwYc UO8XJc')[0].click()"
	},
	{
		url: "*://*.hulu.com/*",
		pausePlay: "Array.from(document.getElementsByClassName('PlayButton')).concat(Array.from( document.getElementsByClassName('PauseButton')))[0].click()",
		pauseCheck: "Array.from(document.getElementsByClassName('PlayButton')).concat(Array.from( document.getElementsByClassName('PauseButton')))[0].classList.contains('PauseButton')",
		next: "document.getElementsByClassName('FastForwardButton')[0].click()",
		prev: "document.getElementsByClassName('RewindButton')[0].click()"
	},
	{
		url: "*://*.amazon.com/gp/video/*",
		pausePlay: "{let mc_player = document.getElementsByTagName('video')[0]; (mc_player.paused?mc_player.play():mc_player.pause())}",
		pauseCheck: "document.getElementsByTagName('video')[0].paused == false",
		next: "document.getElementsByClassName('fastSeekForward')[0].click()",
		prev: "document.getElementsByClassName('fastSeekBack')[0].click()"
	}
];

// Listen for messages
port.onMessage.addListener((message) => {
	// Depending on the message we recieve, take different actions
	switch (message)
	{
		case "togglepause":
			get_media_tab(toggle_pause, 0);
			break;
		case "pause":
			get_media_tab(pause, 0);
			break;
		case "play":
			get_media_tab(play, 0);
			break;
		case "next":
			get_media_tab(next, 0);
			break;
		case "prev":
			get_media_tab(prev, 0);
			break;
	}
});

// Toggle play music pause script on passed tab
function toggle_pause(tab, site)
{
	browser.tabs.executeScript(tab.id, {
		code: site.pausePlay
	});
}

// If the tab isn't yet paused, pause it
function pause(tab, site)
{
	browser.tabs.executeScript(tab.id, {
		code: "if(" + site.pauseCheck + "){" + site.pausePlay + "}"
	});
}

// If the tab isn't yet playing, unpause it
function play(tab, site)
{
	browser.tabs.executeScript(tab.id, {
		code: "if(!(" + site.pauseCheck + ")){" + site.pausePlay + "}"
	});
}

// Go to the next song
function next(tab, site)
{
	browser.tabs.executeScript(tab.id, {
		code: site.next
	});
}

// Go to the previous song
function prev(tab, site)
{
	browser.tabs.executeScript(tab.id, {
		code: site.prev
	});
}

// Find the tab and call the correct action
function get_media_tab(successCallback, siteIndex)
{
	// If we haven't seen any of our sites, we're done
	if(siteIndex == sites.length)
	{
		return;
	}

	// Get our site for this query
	var site = sites[siteIndex];
	// Check if that site exists in our world
	browser.tabs.query({url: site.url}).then
	(
		function(tabs)
		{
			// If we get some tabs, run the action on the first one
			if(tabs.length > 0)
			{
				successCallback(tabs[0], site);
			}
			// If we get no tabs, try the next site in the list
			else
			{
				get_media_tab(successCallback, siteIndex + 1)
			}
		},
		// If something goes wrong, give up
		function(error)
		{
			console.log("Oh no! ", error);
		}
	);
}

// Check once per second for updated audible status
setInterval(function(){ get_media_tab(print_audible_status, 0); }, 1000);

function print_audible_status(tab, site)
{
	// Check if the site is paused
	browser.tabs.executeScript(tab.id, {
		code: site.pauseCheck
	}).then(
		(i) =>
		{
			// Only output if it hasn't changed
			if(i[0] == is_playing)
			{
				return;
			}
			is_playing = i[0];

			// Print out the correct status message to the executable
			if(i[0])
			{
				port.postMessage("Audible");
			}
			else
			{
				port.postMessage("Inaudible");
			}
		}
	);
}
