// Connect to the python server
var port = browser.runtime.connectNative("music_controller");

// The list of sites we want to try, in the order we want
// to try them
var sites = [
	{
		url: "*://play.google.com/music*",
		pauseElement: "document.getElementById('player-bar-play-pause')",
		nextElement: "document.getElementById('player-bar-forward')",
		prevElement: "document.getElementById('player-bar-rewind')",
		pauseCheck: ".title == 'Pause'",
		activate: ".click()"
	},
	{
		url: "*://*.youtube.com/*",
		pauseElement: "document.getElementsByClassName('ytp-play-button')[0]",
		nextElement: "document.getElementsByClassName('ytp-next-button')[0]",
		prevElement: "document.getElementsByClassName('ytp-prev-button')[0]",
		pauseCheck: ".title == 'Pause (k)'",
		activate: ".click()"
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
		code: site.pauseElement + site.activate
	});
}

// If the tab isn't yet paused, pause it
function pause(tab, site)
{
	browser.tabs.executeScript(tab.id, {
		code: "if(" + site.pauseElement + site.pauseCheck + "){" + site.pauseElement + site.activate + "}"
	});
}

// If the tab isn't yet playing, unpause it
function play(tab, site)
{
	browser.tabs.executeScript(tab.id, {
		code: "if(!(" + site.pauseElement + site.pauseCheck + ")){" + site.pauseElement + site.activate + "}"
	});
}

// Go to the next song
function next(tab, site)
{
	browser.tabs.executeScript(tab.id, {
		code: site.nextElement + site.activate
	});
}

// Go to the previous song
function prev(tab, site)
{
	browser.tabs.executeScript(tab.id, {
		code: site.prevElement + site.activate
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
