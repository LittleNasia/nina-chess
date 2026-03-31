using System.Text;

namespace BookGen;

public static class PgnParser
{
	/// <summary>
	/// Streams games from a PGN file one at a time. Each game yields a list of SAN moves.
	/// Only the move text of one game is in memory at a time.
	/// </summary>
	public static IEnumerable<List<string>> StreamGames(string path)
	{
		var moveTextLines = new List<string>();
		bool inMoveText = false;

		foreach (string rawLine in ReadLines(path))
		{
			string line = rawLine.Trim();

			if (line.StartsWith('['))
			{
				if (inMoveText && moveTextLines.Count > 0)
				{
					var moves = TokenizeMoveText(moveTextLines);
					if (moves.Count > 0)
						yield return moves;
					moveTextLines.Clear();
					inMoveText = false;
				}
				continue;
			}

			if (line.Length > 0)
			{
				moveTextLines.Add(line);
				inMoveText = true;
			}
		}

		if (moveTextLines.Count > 0)
		{
			var finalMoves = TokenizeMoveText(moveTextLines);
			if (finalMoves.Count > 0)
				yield return finalMoves;
		}
	}

	private static IEnumerable<string> ReadLines(string path)
	{
		using var reader = new StreamReader(path);
		string? line;
		while ((line = reader.ReadLine()) != null)
			yield return line;
	}

	/// <summary>
	/// Tokenizes move text lines into SAN moves.
	/// Strips comments, variations, NAGs, move numbers, and results.
	/// Operates on the small per-game text, not the whole file.
	/// </summary>
	private static List<string> TokenizeMoveText(List<string> lines)
	{
		var moves = new List<string>();
		int braceCommentDepth = 0;
		int variationDepth = 0;

		foreach (string line in lines)
		{
			var tokens = SplitTokens(line, ref braceCommentDepth, ref variationDepth);
			foreach (string token in tokens)
			{
				string sanitized = token;

				// Strip leading move number (e.g. "1.", "12...", "1.e4" -> "e4")
				int moveNumberEnd = FindMoveNumberEnd(sanitized);
				if (moveNumberEnd > 0)
				{
					sanitized = sanitized[moveNumberEnd..];
					if (sanitized.Length == 0)
						continue;
				}

				// Skip results
				if (sanitized is "1-0" or "0-1" or "1/2-1/2" or "*")
					continue;

				// Skip null moves
				if (sanitized is "--" or "Z0")
					continue;

				// Strip NAGs
				if (sanitized.StartsWith('$'))
					continue;

				if (sanitized.Length > 0)
					moves.Add(sanitized);
			}
		}

		return moves;
	}

	/// <summary>
	/// Splits a line into tokens, skipping characters inside {comments} and (variations).
	/// Tracks brace/paren depth across lines for multi-line comments/variations.
	/// </summary>
	private static List<string> SplitTokens(string line, ref int braceCommentDepth, ref int variationDepth)
	{
		var tokens = new List<string>();
		var currentToken = new StringBuilder();

		for (int charIndex = 0; charIndex < line.Length; charIndex++)
		{
			char character = line[charIndex];

			// Handle brace comments
			if (character == '{') { braceCommentDepth++; FlushToken(currentToken, tokens); continue; }
			if (character == '}') { braceCommentDepth = Math.Max(0, braceCommentDepth - 1); continue; }
			if (braceCommentDepth > 0) continue;

			// Handle line comments
			if (character == ';') break; // rest of line is comment

			// Handle variations
			if (character == '(') { variationDepth++; FlushToken(currentToken, tokens); continue; }
			if (character == ')') { variationDepth = Math.Max(0, variationDepth - 1); continue; }
			if (variationDepth > 0) continue;

			// Normal character
			if (char.IsWhiteSpace(character))
			{
				FlushToken(currentToken, tokens);
			}
			else
			{
				currentToken.Append(character);
			}
		}

		FlushToken(currentToken, tokens);
		return tokens;
	}

	private static void FlushToken(StringBuilder tokenBuilder, List<string> tokens)
	{
		if (tokenBuilder.Length > 0)
		{
			tokens.Add(tokenBuilder.ToString());
			tokenBuilder.Clear();
		}
	}

	/// <summary>
	/// Returns the index past a leading move number prefix like "1.", "12...", "1.".
	/// Returns 0 if the token doesn't start with a move number.
	/// </summary>
	private static int FindMoveNumberEnd(string token)
	{
		int charIndex = 0;
		while (charIndex < token.Length && char.IsDigit(token[charIndex]))
			charIndex++;

		if (charIndex == 0 || charIndex >= token.Length || token[charIndex] != '.')
			return 0;

		while (charIndex < token.Length && token[charIndex] == '.')
			charIndex++;

		return charIndex;
	}
}
