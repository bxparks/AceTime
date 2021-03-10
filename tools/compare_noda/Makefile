json: validation_data.json

validation_data.json: Program.cs small.txt
	dotnet run -- \
		--start_year 2000 \
		--until_year 2038 \
		< small.txt \
		> validation_data.json

clean:
	rm -f validation_data.json
