using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using Microsoft.Win32;

namespace Visualiser;

public partial class MainWindow : Window
{
	// Board colors
	private static readonly Color LightSquareColor = Color.FromRgb(0xF0, 0xD9, 0xB5);
	private static readonly Color DarkSquareColor = Color.FromRgb(0xB5, 0x88, 0x63);

	// Overlay colors (semi-transparent)
	private static readonly SolidColorBrush OverlayMoveFrom = new(Color.FromArgb(140, 0, 160, 50));
	private static readonly SolidColorBrush OverlayMoveTo = new(Color.FromArgb(140, 0, 200, 70));
	private static readonly SolidColorBrush OverlayBlue = new(Color.FromArgb(110, 60, 110, 255));
	private static readonly SolidColorBrush OverlayRed = new(Color.FromArgb(110, 240, 50, 50));
	private static readonly SolidColorBrush OverlayOrange = new(Color.FromArgb(110, 255, 160, 0));
	private static readonly SolidColorBrush OverlayYellow = new(Color.FromArgb(110, 240, 230, 0));
	private static readonly SolidColorBrush OverlayPurple = new(Color.FromArgb(110, 170, 0, 255));
	private static readonly SolidColorBrush OverlayGreen = new(Color.FromArgb(110, 0, 200, 80));

	// Chess piece Unicode characters
	// White (outline): ♔ ♕ ♖ ♗ ♘ ♙
	// Black (filled):  ♚ ♛ ♜ ♝ ♞ ♟
	private static readonly string[] WhitePieceChars = ["\u2659", "\u2658", "\u2657", "\u2656", "\u2655", "\u2654"];
	private static readonly string[] BlackPieceChars = ["\u265F", "\u265E", "\u265D", "\u265C", "\u265B", "\u265A"];

	// Board UI elements - indexed by display order (0 = top-left a8, 63 = bottom-right h1)
	private readonly Border[] _squareOverlays = new Border[64];
	private readonly TextBlock[] _squarePieces = new TextBlock[64];

	// State
	private List<PositionEntry> _entries = [];
	private int _currentIndex = -1;
	private bool _suppressSliderEvent;

	private static readonly string[] OverlayOptions =
	[
		"None",
		"Best Move",
		"Pawn Moves",
		"Knight Moves",
		"Bishop Moves",
		"Rook Moves",
		"Queen Moves",
		"King Moves",
		"Pin Mask",
		"Check Mask",
		"Pinners",
		"Checkers",
		"Attacked Squares",
		"En Passant"
	];

	public MainWindow()
	{
		InitializeComponent();
		CreateBoard();
		OverlayCombo.ItemsSource = OverlayOptions;
		OverlayCombo.SelectedIndex = 0;
		UpdatePositionLabel();
	}

	private void CreateBoard()
	{
		for (int i = 0; i < 64; i++)
		{
			int displayRow = i / 8;
			int displayCol = i % 8;
			bool isLight = (displayRow + displayCol) % 2 == 0;

			var cellGrid = new Grid();
			cellGrid.Background = new SolidColorBrush(isLight ? LightSquareColor : DarkSquareColor);

			var overlay = new Border { Background = Brushes.Transparent };
			cellGrid.Children.Add(overlay);
			_squareOverlays[i] = overlay;

			var piece = new TextBlock
			{
				FontSize = 34,
				FontFamily = new FontFamily("Segoe UI Symbol"),
				HorizontalAlignment = HorizontalAlignment.Center,
				VerticalAlignment = VerticalAlignment.Center,
				Foreground = Brushes.Black
			};
			cellGrid.Children.Add(piece);
			_squarePieces[i] = piece;

			BoardGrid.Children.Add(cellGrid);
		}
	}

	/// <summary>
	/// Converts a display index (0=top-left a8, 63=bottom-right h1) to an engine bit index.
	/// Engine: bit 0 = h1, bit 7 = a1, bit 56 = h8, bit 63 = a8.
	/// </summary>
	private static int DisplayIndexToBitIndex(int displayIndex)
	{
		int displayRow = displayIndex / 8;
		int displayCol = displayIndex % 8;
		return (7 - displayRow) * 8 + (7 - displayCol);
	}

	private void UpdateBoard()
	{
		if (_currentIndex < 0 || _currentIndex >= _entries.Count)
		{
			ClearBoard();
			return;
		}

		var entry = _entries[_currentIndex];
		string selectedOverlay = OverlayCombo.SelectedItem as string ?? "None";

		// Get overlay bitboard and brush for bitboard-based overlays
		var (overlayBits, overlayBrush) = GetOverlayData(entry, selectedOverlay);

		for (int displayIndex = 0; displayIndex < 64; displayIndex++)
		{
			int bitIndex = DisplayIndexToBitIndex(displayIndex);
			ulong bitMask = 1UL << bitIndex;

			// Determine piece on this square
			string pieceChar = "";
			for (int pieceType = 0; pieceType < 6; pieceType++)
			{
				if ((entry.Features[pieceType] & bitMask) != 0)
				{
					pieceChar = WhitePieceChars[pieceType];
					break;
				}
				if ((entry.Features[6 + pieceType] & bitMask) != 0)
				{
					pieceChar = BlackPieceChars[pieceType];
					break;
				}
			}
			_squarePieces[displayIndex].Text = pieceChar;

			// Determine overlay highlight
			if (selectedOverlay == "Best Move")
			{
				if (bitIndex == entry.MoveFrom)
					_squareOverlays[displayIndex].Background = OverlayMoveFrom;
				else if (bitIndex == entry.MoveTo)
					_squareOverlays[displayIndex].Background = OverlayMoveTo;
				else
					_squareOverlays[displayIndex].Background = Brushes.Transparent;
			}
			else if (overlayBits != 0 && (overlayBits & bitMask) != 0)
			{
				_squareOverlays[displayIndex].Background = overlayBrush;
			}
			else
			{
				_squareOverlays[displayIndex].Background = Brushes.Transparent;
			}
		}
	}

	private static (ulong bits, SolidColorBrush brush) GetOverlayData(PositionEntry entry, string overlay)
	{
		return overlay switch
		{
			"Pawn Moves" => (entry.Features[PositionEntry.PawnMovesIndex], OverlayBlue),
			"Knight Moves" => (entry.Features[PositionEntry.KnightMovesIndex], OverlayBlue),
			"Bishop Moves" => (entry.Features[PositionEntry.BishopMovesIndex], OverlayBlue),
			"Rook Moves" => (entry.Features[PositionEntry.RookMovesIndex], OverlayBlue),
			"Queen Moves" => (entry.Features[PositionEntry.QueenMovesIndex], OverlayBlue),
			"King Moves" => (entry.Features[PositionEntry.KingMovesIndex], OverlayBlue),
			"Pin Mask" => (entry.Features[PositionEntry.PinmaskIndex], OverlayOrange),
			"Check Mask" => (entry.Features[PositionEntry.CheckmaskIndex], OverlayYellow),
			"Pinners" => (entry.Features[PositionEntry.PinnersIndex], OverlayPurple),
			"Checkers" => (entry.Features[PositionEntry.CheckersIndex], OverlayRed),
			"Attacked Squares" => (entry.Features[PositionEntry.AttackedSquaresIndex], OverlayRed),
			"En Passant" => (entry.Features[PositionEntry.EnPassantIndex], OverlayGreen),
			_ => (0UL, OverlayBlue)
		};
	}

	private void ClearBoard()
	{
		for (int i = 0; i < 64; i++)
		{
			_squarePieces[i].Text = "";
			_squareOverlays[i].Background = Brushes.Transparent;
		}
	}

	private void UpdateInfoPanel()
	{
		if (_currentIndex < 0 || _currentIndex >= _entries.Count)
		{
			SideToMoveText.Text = "-";
			ScoreText.Text = "-";
			BestMoveText.Text = "-";
			MoveDetailsText.Text = "-";
			ResultText.Text = "-";
			CastlingText.Text = "-";
			EnPassantText.Text = "-";
			InCheckText.Text = "-";
			return;
		}

		var entry = _entries[_currentIndex];

		SideToMoveText.Text = entry.SideToMove == 0 ? "White" : "Black";
		ScoreText.Text = entry.FormatScore();
		BestMoveText.Text = $"{entry.MoveToUci()}  ({PositionEntry.SquareName(entry.MoveFrom)} \u2192 {PositionEntry.SquareName(entry.MoveTo)})";
		MoveDetailsText.Text = $"{PositionEntry.PieceTypeName(entry.MovePiece)}, {PositionEntry.MoveTypeName(entry.MoveType)}";
		ResultText.Text = PositionEntry.ResultName(entry.Result);
		CastlingText.Text = entry.CastlingRights();

		ulong epSquare = entry.Features[PositionEntry.EnPassantIndex];
		EnPassantText.Text = epSquare != 0
			? PositionEntry.SquareName(PositionEntry.BitScanForward(epSquare))
			: "None";

		bool inCheck = entry.Features[PositionEntry.CheckersIndex] != 0;
		InCheckText.Text = inCheck ? "Yes" : "No";
		InCheckText.Foreground = inCheck ? Brushes.Red : Brushes.Black;
	}

	private void UpdatePositionLabel()
	{
		if (_entries.Count == 0)
			PositionLabel.Text = "No file loaded";
		else
			PositionLabel.Text = $"{_currentIndex + 1} / {_entries.Count}";
	}

	private void NavigateTo(int index)
	{
		if (_entries.Count == 0) return;

		index = Math.Clamp(index, 0, _entries.Count - 1);
		if (index == _currentIndex) return;

		_currentIndex = index;

		_suppressSliderEvent = true;
		PositionSlider.Value = index;
		_suppressSliderEvent = false;

		UpdateBoard();
		UpdateInfoPanel();
		UpdatePositionLabel();
	}

	// --- Event handlers ---

	private void OpenFile_Click(object sender, RoutedEventArgs e)
	{
		var dialog = new OpenFileDialog
		{
			Filter = "Binary files (*.bin)|*.bin|All files (*.*)|*.*",
			Title = "Open Game Generation Data"
		};

		if (dialog.ShowDialog() != true) return;

		try
		{
			_entries = PositionEntry.LoadFromFile(dialog.FileName);
			_currentIndex = -1;

			PositionSlider.Maximum = Math.Max(0, _entries.Count - 1);
			PositionSlider.Value = 0;

			FileInfoText.Text = dialog.FileName;
			TotalPositionsText.Text = _entries.Count.ToString("N0");

			if (_entries.Count > 0)
			{
				NavigateTo(0);
			}
			else
			{
				UpdateBoard();
				UpdateInfoPanel();
				UpdatePositionLabel();
			}
		}
		catch (Exception ex)
		{
			MessageBox.Show($"Error loading file:\n{ex.Message}", "Error",
				MessageBoxButton.OK, MessageBoxImage.Error);
		}
	}

	private void First_Click(object sender, RoutedEventArgs e) => NavigateTo(0);
	private void Previous_Click(object sender, RoutedEventArgs e) => NavigateTo(_currentIndex - 1);
	private void Next_Click(object sender, RoutedEventArgs e) => NavigateTo(_currentIndex + 1);
	private void Last_Click(object sender, RoutedEventArgs e) => NavigateTo(_entries.Count - 1);

	private void Slider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
	{
		if (_suppressSliderEvent) return;
		NavigateTo((int)e.NewValue);
	}

	private void Overlay_Changed(object sender, SelectionChangedEventArgs e)
	{
		UpdateBoard();
	}

	private void Window_KeyDown(object sender, KeyEventArgs e)
	{
		switch (e.Key)
		{
			case Key.Left:
				NavigateTo(_currentIndex - 1);
				e.Handled = true;
				break;
			case Key.Right:
				NavigateTo(_currentIndex + 1);
				e.Handled = true;
				break;
			case Key.Home:
				NavigateTo(0);
				e.Handled = true;
				break;
			case Key.End:
				NavigateTo(_entries.Count - 1);
				e.Handled = true;
				break;
			case Key.PageUp:
				NavigateTo(_currentIndex - 100);
				e.Handled = true;
				break;
			case Key.PageDown:
				NavigateTo(_currentIndex + 100);
				e.Handled = true;
				break;
		}
	}
}
