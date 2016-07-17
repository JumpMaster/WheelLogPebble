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
				"type": "select",
				"messageKey": "horn_mode",
				"label": "Horn Mode",
				"description": "Onboard, via phone, or offUse the onboard horn or via bluetooth audio?",
				"defaultValue": "On board",
				"options": [
					{ 
						"label": "Off", 
						"value": "0"
					},
					{ 
						"label": "On board",
						"value": "1"
					},
					{ 
						"label": "Phone",
						"value": "2"
					}
					]
			}
		]
	},
	{
		"type": "submit",
		"defaultValue": "Save"
	}
];