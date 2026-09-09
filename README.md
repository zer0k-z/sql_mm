# SQLMM

SQLMM provides simple non-blocking MySQL/SQLite connectors for MetaMod.

> [!IMPORTANT]
> Linux builds require the `default-libmysqlclient-dev` package to be installed.

# Features

- Non-blocking MySQL/SQLite queries/transactions
- Windows & Linux support
- Simple API

# Interface

SQLMM will expose an interface in `OnMetamodQuery` which can then be queried with `(ISQLInterface*)g_SMAPI->MetaFactory(SQLMM_INTERFACE, &ret, NULL);` by other plugins.
Example:
```c++
  ISQLiteClient *g_pSQLiteClient;
	g_pSQLiteClient = ((ISQLInterface *)g_SMAPI->MetaFactory(SQLMM_INTERFACE, nullptr, nullptr))->GetSQLiteClient();
	g_SMAPI->PathFormat(buffer, sizeof(buffer), "path/to/database/db.sqlite3");
	if (g_pSQLiteClient)
	{
		SQLiteConnectionInfo info;
		info.database = buffer;
		conn = g_pSQLiteClient->CreateSQLiteConnection(info);
		conn->Connect(
			[](bool success)
			{
        //...
			});
	}
```

Interface definition can be found in `src/public`.

## Compilation

### Requirements

- [Metamod:Source](https://www.sourcemm.net/downloads.php/?branch=master) 2.0.0.1462 or higher, which uses
  [KHook](https://github.com/Kenzzer/KHook) instead of SourceHook. Clone it with `--recursive`; KHook is a submodule
  of it and its headers are needed to build.
- [AMBuild](https://wiki.alliedmods.net/Ambuild)
- On Linux, the `default-libmysqlclient-dev` and `lld` packages.

```bash
git clone https://github.com/zer0k-z/sql_mm && cd sql_mm
git submodule update --init

export MMSOURCE20=/path/to/metamod-source/
export HL2SDKCS2=/path/to/hl2sdk-cs2

mkdir build && cd build
python3 ../configure.py -s cs2
ambuild
```

The package is written to `build/package`.
