module.exports = [
	{ 
		"type": "heading", 
		"defaultValue": "WheelLog" 
	}, 
	{ 
		"type": "text", 
		"defaultValue": "A dashboard for KingSong and Gotway electric unicycles" 
	},
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"defaultValue": "Speed Settings"
			},
			{
				"type": "slider",
				"messageKey": "max_speed",
				"defaultValue": 30,
				"label": "Max Speed",
				"description": "The maximum speed for the display",
				"min": 0,
				"max": 50,
				"step": 0.5
			},
			{
				"type": "slider",
				"messageKey": "vibe_speed",
				"defaultValue": 28,
				"label": "Vibrate Speed",
				"description": "The speed which triggers the watch to vibrate. 0 is off.",
				"min": 0,
				"max": 50,
				"step": 0.5
			},
			{
				"type": "toggle",
				"messageKey": "use_onboard_horn",
				"label": "Use Onboard Horn",
				"description": "Use the onboard horn or via bluetooth audio?",
			}
		]
	},
	{
		"type": "submit",
		"defaultValue": "Save"
	}
];