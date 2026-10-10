int regression_function(int value);

int regression_function(int value)
{
    return value;
}

int main(void)
{
    int local_prototype(int value);
    int (*callback)(int) = regression_function;
    return callback(0);
}