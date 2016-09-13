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
				"type": "toggle",
				"messageKey": "speed_in_mph",
				"defaultValue": false,
				"label": "Show Speed in MPH",
				"description": "Show the speed in MPH rather than KPH",
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
						"label": "Via Bluetooth Audio",
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