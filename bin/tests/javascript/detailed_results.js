$(function()
{
	//these arguments are declared in two other files output by the Tournament Manager
	fillFilters(resultsSummary, maps);
	
	$("select.filter").change(function()
	{
		fillDetailedResultsTable(detailedResults, replayPath);
		$("#detailedResultsTable").tablesorter();
	});
	
	fillDetailedResultsTable(detailedResults, replayPath);
	$("#detailedResultsTable").tablesorter();
	
	// toggle the legend div
	$("#legend-toggle").click(function(event) {
		if ($(event.currentTarget).html() === "Show Table Legend") {
			$(event.currentTarget).html('Hide Table Legend');
			$("#legend").show();
		}
		else {
			$(event.currentTarget).html('Show Table Legend');
			$("#legend").hide();
		}
	});
});

function fillFilters(resultsSummary, maps)
{
	for (var i = 0; i < resultsSummary.length; i++)
	{
		$("#bots").append("<option value='" + resultsSummary[i].BotName + "'>" + resultsSummary[i].BotName + "</option>");
		$("#winner").append("<option value='" + resultsSummary[i].BotName + "'>" + resultsSummary[i].BotName + "</option>");
		$("#loser").append("<option value='" + resultsSummary[i].BotName + "'>" + resultsSummary[i].BotName + "</option>");
	}
	
	for (var i = 0; i < maps.length; i++)
	{
		$("#maps").append("<option value='" + maps[i] + "'>" + maps[i] + "</option>");
	}
}

function filterResult(result, crashFilter, botFilter, winnerFilter, loserFilter, mapFilter)
{
	if (crashFilter == "only-crashes" && result.crash == -1)
	{
		return false;
	}
	if (crashFilter == "no-crashes" && result.crash != -1)
	{
		return false;
	}
	if (botFilter != "all" && result.bots.indexOf(botFilter) == -1)
	{
		return false;
	}
	if (winnerFilter != "all" && winnerFilter != result.bots[result.winner])
	{
		return false;
	}
	if (loserFilter != "all" && loserFilter != result.bots[(result.winner + 1) % 2])
	{
		return false;
	}
	if (mapFilter != "all" && mapFilter != result.map)
	{
		return false;
	}
	return true;
}

function fillDetailedResultsTable(data, replayDir)
{
	var crashFilter = $("#crashes").val();
	var botFilter = $("#bots").val();
	var winnerFilter = $("#winner").val();
	var loserFilter = $("#loser").val();
	var mapFilter = $("#maps").val();
	var unfilteredGames = 0;
	
	var html = "";
	
	var winnerTimerHeaders = "";
	var loserTimerHeaders = "";
	
	var numResults = data.length;
	if (numResults > 0)
	{
		var numTimers = data[0].timers[0].length;
		for (var i = 0; i < numTimers; i++)
		{
			winnerTimerHeaders += "<th> W " + data[0].timers[0][i].timeInMS + "ms<br>(Max " + frameLimits[i].frameCount + ")</th>";
			loserTimerHeaders += "<th> L " + data[0].timers[0][i].timeInMS + "ms<br>(Max " + frameLimits[i].frameCount + ")</th>";
		}
	}
	
	//headers
	var headerHtml = "<tr><th>Game ID</th><th>Round</th><th>Winner</th><th>Loser</th><th>Crash</th><th>Timeout</th><th>Map</th><th>Duration</th><th>End Type</th><th>W Score</th><th>L Score</th><th>(W-L) / Max</th>";
	headerHtml += winnerTimerHeaders + loserTimerHeaders;
	headerHtml += "<th>Win Addr</th><th>Lose Addr</th><th>Start</th><th>Finish</th></tr>";
	
	$("#detailedResultsTable thead").html(headerHtml);
	
	for (var i=0; i<numResults; ++i)
	{
		if (data[i].winner == -1)
		{
			if (data[i].gameEndType == "NO_REPORT")
			{
				data[i].winner = 0;
				data[i].crash = 1;
			}
			else if (data[i].crash != -1)
			{
				data[i].winner = (data[i].crash + 1) % 2;
			}
			else if (data[i].duration == "00:00:00")
			{
				data[i].winner = 0;
			}
			else
			{
				data[i].winner = 1;
			}
		}
		
		//check filters
		if (!filterResult(data[i], crashFilter, botFilter, winnerFilter, loserFilter, mapFilter))
		{
			continue;
		}
		unfilteredGames += 1;
		
		html += "<tr>";
		html += "<td>"+ data[i].gameID + "</td>"; 
		html += "<td>"+ data[i].round + "</td>";
		
		var winner = data[i].winner;
		var loser = (data[i].winner + 1) % 2;
		
		if (data[i].replays[winner] != "")
		{
			html += "<td><a href='" + replayDir + data[i].replays[winner] + "'>" + data[i].bots[winner] + "</a></td>";
		}
		else
		{
			html += "<td>" + data[i].bots[winner] + "</td>";
		}
		
		if (data[i].replays[loser] != "")
		{
			html += "<td><a href='" + replayDir + data[i].replays[loser] + "'>" + data[i].bots[loser] + "</a></td>";
		}
		else
		{
			html += "<td>" + data[i].bots[loser] + "</td>";
		}
		
		html += "<td>" + (data[i].crash == -1 ? "" : data[i].bots[data[i].crash]) + "</td>";
		html += "<td>" + (data[i].timeout == -1 ? "" : data[i].bots[data[i].timeout]) + "</td>";
		html += "<td>" + data[i].map + "</td>";
		html += "<td>" + data[i].duration + "</td>";
		html += "<td>" + data[i].gameEndType + "</td>";
		html += "<td>" + data[i].scores[winner] + "</td>";
		html += "<td>" + data[i].scores[loser] + "</td>";
		html += "<td>" + data[i]["(W-L)/Max"] + "</td>";
		
		for (var j = 0; j < numTimers; j++)
		{
			html += "<td>" + data[i].timers[winner][j].frameCount + "</td>";
		}
		for (var j = 0; j < numTimers; j++)
		{
			html += "<td>" + data[i].timers[loser][j].frameCount + "</td>";
		}
		
		html += "<td>" + data[i].addresses[winner] + "</td>";
		html += "<td>" + data[i].addresses[loser] + "</td>";
		html += "<td>" + data[i].start + "</td>";
		html += "<td>" + data[i].finish + "</td>";
		
		html += "</tr>";
	}
	
	$("#gameCount").html("Games: " + unfilteredGames);
	$("#detailedResultsTable tbody").html(html);
}