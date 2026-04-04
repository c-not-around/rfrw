unit UBoolean;


static function boolean.operator +(left, right: boolean) := left or right;

static function boolean.operator -(left, right: boolean) := not (left or right);

static function boolean.operator *(left, right: boolean) := left and right;

static function boolean.operator /(left, right: boolean) := not (left and right);


static procedure boolean.operator +=(var left: boolean; right: boolean) := left := left or right;

static procedure boolean.operator -=(var left: boolean; right: boolean) := left := not (left or right);

static procedure boolean.operator *=(var left: boolean; right: boolean) := left := left and right;

static procedure boolean.operator /=(var left: boolean; right: boolean) := left := not (left and right);


static function boolean.operator implicit(b: boolean): integer := b ? 1 : 0;

static function boolean.operator -(b: boolean) := not b;


end.