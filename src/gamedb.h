#ifndef GAMEDB_H
#define GAMEDB_H

#include <QAbstractItemModel>
#include <QMutex>
#include <QString>
#include <vector>
#include <atomic>

#include "bitarray.h"
#include "coords.h"

class go_pattern;

struct gamedb_entry
{
	QString dirname, filename;
	QString pw, pb;
	QString date, result, event;
	std::vector<char> movelist;
	int id;
	int boardsize;
	bit_array finalpos_w, finalpos_b, finalpos_c;
	size_t movelist_off = 0;

	gamedb_entry (int i, const QString &dir, const QString &f, const QString &w, const QString &b,
		      const QString &d, const QString &r, const QString &e, int sz)
		: dirname (dir), filename (f), pw (w), pb (b), date (d), result (r), event (e),
		id (i), boardsize (sz), finalpos_w (sz * sz), finalpos_b (sz * sz), finalpos_c (sz * sz)
	{
	}
	gamedb_entry (const gamedb_entry &other) = default;
	gamedb_entry (gamedb_entry &&other) = default;
	gamedb_entry &operator =(gamedb_entry &&other) = default;
	gamedb_entry &operator =(const gamedb_entry &other) = default;
};

class gamedb_model : public QAbstractItemModel
{
	Q_OBJECT

	std::vector<unsigned> m_entries;
	void default_sort ();
public:
	using cont_bw = std::pair<int, int>;
	gamedb_model ();
	void clear_list ();
	void apply_filter (const QString &p1, const QString &p2, const QString &event,
			   const QString &dtfrom, const QString &dtto);
	void apply_game_list (std::vector<unsigned> &&);
	void reset_filters ();
	using search_result = std::pair<bit_array, std::vector<cont_bw>>;
	search_result find_pattern (const go_pattern &, std::atomic<long> *, std::atomic<long> *);
	const gamedb_entry &find (size_t) const;
	QString status_string () const;

	virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QModelIndex index (int row, int col, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent (const QModelIndex &index ) const override;
	int rowCount (const QModelIndex &parent = QModelIndex()) const override;
	int columnCount (const QModelIndex &parent = QModelIndex()) const override;
	QVariant headerData (int section, Qt::Orientation orientation,
			     int role = Qt::DisplayRole) const override;

	// Qt::ItemFlags flags(const QModelIndex &index) const override;
public slots:
	void slot_prepare_load ();
	void slot_load_complete ();
signals:
	void signal_changed ();
};

class GameDB_Data_Controller : public QObject
{
	Q_OBJECT

public:
	GameDB_Data_Controller ();
	void load ();
signals:
	void signal_prepare_load ();
	void signal_start_load ();
};

class GameDB_Data : public QObject
{
	Q_OBJECT

	void do_load ();
public:
	/* A local copy of the paths in settings.  */
	QStringList dbpaths;
	/* Computed in slot_start_load.  Once completed, we signal_load_complete,
	   after which only the main thread can access this vector.  */
	std::vector<gamedb_entry> m_all_entries;
	QMutex db_mutex;
	bool load_complete = false;

	bool read_extra_file (QDataStream &, int, int);

public slots:
	void slot_start_load ();
signals:
	void signal_load_complete ();
};

extern GameDB_Data *db_data;

extern bit_array match_games (const std::vector<unsigned> &, const go_pattern &,
			      std::vector<gamedb_model::cont_bw> &conts, coord_transform);

#endif
