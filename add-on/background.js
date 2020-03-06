// Connect to the python server
var port = browser.runtime.connectNative("music_controller");

// Listen for messages
port.onMessage.addListener((message) => {
	// If we get the pause message, get the tab with
	// play music, and call toggle_pause on it
	if(message == "pause")
	{
		browser.tabs.query({url: "*://play.google.com/music*"}).then
		(
			function(tabs)
			{
				toggle_pause(tabs[0])
			},
			function(error)
			{
				console.log("Oh no! ", error);
			}
		);
	}
});

// Toggle play music pause script on passed tab
function toggle_pause(tab)
{
	browser.tabs.executeScript(tab.id, {
		code: "document.getElementById('player-bar-play-pause').click();"
	});
}
