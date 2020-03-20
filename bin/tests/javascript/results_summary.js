$(function()
{
	//these arguments are declared in another file output by the Tournament Manager
	displayDetailedResultsSize(resultsTXTSize)
	fillResultsTable(resultsSummary);
	fillPairsTable(resultsSummary);
	fillMapsTable(resultsSummary, maps);
	displayUpdateTime(lastUpdateTime);
});

function displayDetailedResultsSize(size)
{
	$("#plainTextSize").html("(" + size + " kB)");
}

function fillResultsTable(data)
{
	var dataTotals = [0, 0, 0, 0, 0, 0, 0, 0];
	var html = "";
	for (var i=0; i<data.length; ++i)
	{
		html += "<tr>";
		html += "<td class=\"race-" + data[i].Race.toLowerCase() + "\">"+ data[i].BotName + "</td>"; 
		html += "<td>" + data[i].Games + "</td>";
		html += "<td>" + data[i].Wins + "</td>";
		html += "<td>" + data[i].Losses + "</td>";
		html += "<td>" + data[i].Score + "</td>";		
		//html += "<td>" + data[i].ELO + "</td>";
		html += "<td>" + getTime(data[i].AvgTime) + "</td>";
		html += "<td>" + getWallTime(data[i].WallTime) + "</td>";
		html += "<td>" + data[i].Hour + "</td>";
		html += "<td>" + data[i].Crash + "</td>";
		html += "<td>" + data[i].Timeout + "</td>";
		html += "</tr>";
		
		dataTotals[0] += data[i].Games;
		dataTotals[1] += data[i].Wins;
		dataTotals[2] += data[i].Losses;
		dataTotals[3] += data[i].AvgTime;
		dataTotals[4] += data[i].WallTime;
		dataTotals[5] += data[i].Hour;
		dataTotals[6] += data[i].Crash;
		dataTotals[7] += data[i].Timeout;
		
		$("#resultsTable tbody").html(html);
	}
	
	html = "<td>Total</td>";
	html += "<td>" + (dataTotals[0]/2) + "</td>";
	html += "<td>" + (dataTotals[1]) + "</td>";
	html += "<td>" + (dataTotals[2]) + "</td>";
	html += "<td>" + "N/A" + "</td>";
	//html += "<td>" + "N/A" + "</td>"; // ELO
	html += "<td>" + getTime((dataTotals[3]/data.length)) + "</td>";
	html += "<td>" + getWallTime(Math.round(dataTotals[4]/data.length)) + "</td>";
	html += "<td>" + (dataTotals[5]/2) + "</td>";
	html += "<td>" + (dataTotals[6]) + "</td>";
	html += "<td>" + (dataTotals[7]) + "</td>";
	
	$("#resultsTable tfoot tr").html(html);
}

function fillPairsTable(data)
{
	var html = "<th>Bot</th><th>Win %</th>";
	
	for (var i=0; i<data.length; i++)
	{
		html += "<th class='race-" + data[i].Race.toLowerCase() + "'>" + data[i].BotName.substr(0, Math.min(4, data[i].BotName.length)) + "</th>";
	}
	
	$("#resultsPairTable thead tr").html(html);
	
	html = "";
	
	for (var i=0; i<data.length; i++)
	{	
		html += "<tr><td class='race-" + data[i].Race.toLowerCase() + "'>" + data[i].BotName + "</td>";
		html += "<td>" + data[i].Score + "</td>";
			
		for (bot in data[i].resultPairs)
		{
			var resultPair = data[i].resultPairs[bot];
			if (resultPair.Opponent == data[i].BotName)
			{
				html += "<td style='background-color: #ffffff'>-</td>";
			}
			else
			{
				html += getWinGameRatioCell(resultPair.Wins, resultPair.Games);
			}
		}
		html += "</tr>";
	}
	$("#resultsPairTable tbody").html(html);
}

function fillMapsTable(data, maps)
{
	var html = "<th>Bot</th>";
	for (var i=0; i<maps.length; i++)
	{
		html += "<th>" + maps[i].substring(0, Math.min(7, maps[i].length)) + "</th>";
	}
	
	$("#resultsMapTable thead tr").html(html);
	
	html = "";
	for (var i=0; i<data.length; i++)
	{
		html += "<tr>";
		html += "<td class='race-" + data[i].Race.toLowerCase() + "'>" + data[i].BotName + "</td>";
		
		for (var j=0; j<maps.length; j++)
		{
			html += getWinGameRatioCell(data[i].mapResults[j].Wins, data[i].mapResults[j].Games)	
		}
		html += "</tr>";
	}
	
	$("#resultsMapTable tbody").html(html);
}

function displayUpdateTime(time)
{
	$("#footer").html("Last updated: " + time);
}

function displayFileSizes(size)
{
	
}

//calculates game time at normal speed using frame count
function getTime(frames)
{
	var fpm = 24*60;
	var fps = 24;
	var minutes = Math.floor(frames / fpm);
	var seconds = Math.floor((frames / fps) % 60);
	
	return "" + minutes + ":" + (seconds < 10 ? "0" + seconds : seconds);
}

//calculates game time at normal speed using frame count
function getWallTime(seconds)
{
	var minutes = Math.floor(seconds / 60);
	seconds -= minutes * 60;
	
	return minutes + ":" + (seconds < 10 ? "0" + seconds : seconds);
}

//returns "<td>....</td>" with wins/games as content with cell color according to ratio
function getWinGameRatioCell(w, g)
{
	//sensitivity of color change
	var k = 1.7;
	
	var l = g - w;
	var p = g > 0 ? w / g : 0;
	var c = Math.floor(p * 255);
	
	if (w >= l)
	{
		c = 255 - c;
		c = Math.floor(k * c);
		var cellColor = "rgb(" + c + "," + 255 + "," + c + ")";
	}
	else
	{
		c = Math.floor(k * c);
		var cellColor = "rgb(" + 255 + "," + c + "," + c + ")";
	}
	
	//convert to string and add leading zeroes (at least size 2)
	w = w + "";
	g = g + "";
	if (w.length < 2)
	{
		w = "0" + w;
	}
	var digits = Math.max(w.length, g.length)
	while (w.length < digits)
	{
		w = "0" + w;
	}
	while (g.length < digits)
	{
		g = "0" + g;
	}
	
	return "<td style='background-color:" + cellColor + "'>" + w + "/" + g + "</td>";
}