$COPYRIGHT_LINE$


$MY_HEADER_INCLUDE_DIRECTIVE$


// Sets default values for this component's properties
$PREFIX$$UNPREFIXED_CLASS_NAME$::$PREFIX$$UNPREFIXED_CLASS_NAME$()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void $PREFIX$$UNPREFIXED_CLASS_NAME$::BeginPlay()
{
	Super::BeginPlay();

	// ...
	$END$
}


// Called every frame
void $PREFIX$$UNPREFIXED_CLASS_NAME$::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

