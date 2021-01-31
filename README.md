# My-Tar

Custom My Tar implementation of tar archive with following functionalities:
- Create a new archive containing the specified items
- Appending items to archive
- List archive contents
- Extract to disk from the archive
- Error logging

More specific info: 

```-c``` Create a new archive containing the specified items.<br/>
```-r``` Like -c, but new entries are appended to the archive. The -f option is required.<br/>
```-t``` List archive contents to stdout.<br/>
```-u``` Like -r, but new entries are added only if they have a modification date newer than the corresponding entry in the archive. The -f option is required.<br/>
```-x``` Extract to disk from the archive. If a file with the same name appears more than once in the archive, each copy will be extracted, with later copies overwriting (replacing) earlier copies. <br/>
 ### Example: 
 ```./my_tar -cf file.tar source.c source.h```
